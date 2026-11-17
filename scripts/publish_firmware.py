"""Publish verified firmware assets into the separate public distribution checkout."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import uuid
import zipfile


def publish(assets: Path, output: Path, metadata: dict, repository: str):
    version = metadata['tag_name']
    if not re.fullmatch(r'v\d+\.\d+\.\d+', version) or metadata.get('draft') or metadata.get('prerelease'):
        raise ValueError('Only published stable version tags can be distributed')
    manifest = json.loads((assets / 'manifest.json').read_text(encoding='utf-8-sig'))
    if manifest['version'] != version or not manifest['firmware']:
        raise ValueError('Manifest does not match release')
    expected = {}
    for line in (assets / 'SHA256SUMS').read_text(encoding='utf-8-sig').splitlines():
        digest, name = line.split(maxsplit=1)
        expected[name.lstrip('*')] = digest.lower()
    names = {'manifest.json', 'SHA256SUMS'}
    for profile in manifest['firmware']:
        if not re.fullmatch(r'c3_(?:transponder|decoder_(?:minimal|oled|w5500|lengkap))', profile['id']):
            raise ValueError('Unknown firmware profile')
        if profile['chipFamily'] != 'ESP32-C3' or {p['address'].lower() for p in profile['parts']} != {'0x0', '0x8000', '0xe000', '0x10000'}:
            raise ValueError('Incomplete ESP32-C3 firmware')
        for part in profile['parts']:
            name = part['asset']
            if not re.fullmatch(re.escape(profile['id']) + r'-(bootloader|partitions|boot_app0|firmware)\.bin', name):
                raise ValueError('Unexpected firmware filename')
            if hashlib.sha256((assets / name).read_bytes()).hexdigest() != expected.get(name):
                raise ValueError('Firmware checksum mismatch: ' + name)
            names.add(name)
    index_path = output / 'releases.json'
    index = json.loads(index_path.read_text()) if index_path.exists() else []
    destination = output / version
    if destination.exists():
        existing = next((r for r in index if r['tag_name'] == version), None)
        previous_manifest = destination / 'manifest.json'
        if not existing or not previous_manifest.exists() or json.loads(previous_manifest.read_text(encoding='utf-8-sig')) != manifest:
            raise ValueError('Published firmware versions are immutable')
        for name in names - {'manifest.json', 'SHA256SUMS'}:
            if not (destination / name).exists() or (destination / name).read_bytes() != (assets / name).read_bytes():
                raise ValueError('Published firmware versions are immutable')
        return existing
    output.mkdir(parents=True, exist_ok=True)
    # Stage and validate everything before replacing the version or public index.
    staged = output / ('.staging-' + uuid.uuid4().hex)
    staged.mkdir()
    try:
        for name in sorted(names):
            shutil.copyfile(assets / name, staged / name)
        for profile in manifest['firmware']:
            archive_name = profile['id'] + '.zip'
            with zipfile.ZipFile(staged / archive_name, 'w', zipfile.ZIP_DEFLATED) as archive:
                archive.writestr('manifest.json', json.dumps({**manifest, 'firmware': [profile]}, indent=2))
                profile_names = [p['asset'] for p in profile['parts']]
                archive.writestr('SHA256SUMS', '\n'.join(f'{expected[name]}  {name}' for name in profile_names) + '\n')
                for name in profile_names:
                    archive.write(staged / name, name)
            names.add(archive_name)
        base = f'https://raw.githubusercontent.com/{repository}/main/{version}'
        release = {
            'id': metadata['id'], 'tag_name': version, 'name': metadata.get('name') or version,
            'published_at': metadata.get('published_at'), 'body': metadata.get('body') or '',
            'html_url': f'https://github.com/{repository}/tree/main/{version}',
            'prerelease': False, 'draft': False,
            'assets': [{'name': name, 'browser_download_url': f'{base}/{name}', 'size': (staged / name).stat().st_size} for name in sorted(names)],
        }
        index = [r for r in index if r['tag_name'] != version] + [release]
        index.sort(key=lambda r: tuple(map(int, r['tag_name'][1:].split('.'))), reverse=True)
        destination.mkdir(exist_ok=True)
        for name in sorted(names):
            shutil.copyfile(staged / name, destination / name)
        next_index = output / 'releases.json.tmp'
        next_index.write_text(json.dumps(index, indent=2) + '\n', encoding='utf-8')
        next_index.replace(index_path)
    finally:
        shutil.rmtree(staged)
    return release


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--assets', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--metadata', type=Path, required=True)
    parser.add_argument('--repository', default='LuminixDevs/idlap-firmware')
    args = parser.parse_args()
    release = publish(args.assets, args.output, json.loads(args.metadata.read_text(encoding='utf-8-sig')), args.repository)
    print('Published public firmware assets:', release['tag_name'])
