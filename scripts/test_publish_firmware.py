import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import unittest
import uuid
import zipfile

MODULE = Path(__file__).with_name('publish_firmware.py')


class PublishTests(unittest.TestCase):
    def setUp(self):
        self.assertTrue(MODULE.exists(), 'Public firmware publisher is not implemented')
        spec = importlib.util.spec_from_file_location('publisher', MODULE)
        self.publisher = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.publisher)
        self.root = Path(__file__).resolve().parents[1] / '.pio' / ('publisher-test-' + uuid.uuid4().hex)
        self.root.mkdir(parents=True)
        self.addCleanup(shutil.rmtree, self.root)
        self.assets = self.root / 'assets'
        self.assets.mkdir()
        self.output = self.root / 'public'
        parts = [{'address': address, 'asset': f'c3_transponder-{name}.bin'} for address, name in
                 [('0x0', 'bootloader'), ('0x8000', 'partitions'), ('0xe000', 'boot_app0'), ('0x10000', 'firmware')]]
        self.manifest = {'schemaVersion': 1, 'version': 'v1.0.0', 'generatedAt': '2026-11-17T00:00:00Z',
                         'firmware': [{'id': 'c3_transponder', 'name': 'idLap Transponder', 'type': 'transponder', 'chipFamily': 'ESP32-C3', 'parts': parts}]}
        (self.assets / 'manifest.json').write_text(json.dumps(self.manifest))
        for part in parts:
            (self.assets / part['asset']).write_bytes(part['asset'].encode())
        self.write_checksums()
        self.metadata = {'id': 1, 'tag_name': 'v1.0.0', 'name': 'idLap v1.0.0', 'published_at': '2026-11-17T00:00:00Z', 'body': 'Release notes'}

    def write_checksums(self):
        (self.assets / 'SHA256SUMS').write_text('\n'.join(f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}' for p in self.assets.glob('*.bin')))

    def publish(self):
        return self.publisher.publish(self.assets, self.output, self.metadata, 'LuminixDevs/idlap-firmware')

    def test_publishes_profile_zip_and_public_index_without_source(self):
        (self.assets / 'secret.cpp').write_text('PRIVATE SOURCE')
        self.publish()
        index = json.loads((self.output / 'releases.json').read_text())
        self.assertEqual(index[0]['tag_name'], 'v1.0.0')
        self.assertTrue(all(a['browser_download_url'].startswith('https://raw.githubusercontent.com/LuminixDevs/idlap-firmware/main/v1.0.0/') for a in index[0]['assets']))
        self.assertFalse((self.output / 'v1.0.0/secret.cpp').exists())
        with zipfile.ZipFile(self.output / 'v1.0.0/c3_transponder.zip') as archive:
            self.assertEqual(set(archive.namelist()), {'manifest.json', 'SHA256SUMS', *(part['asset'] for part in self.manifest['firmware'][0]['parts'])})

    def test_preserves_other_releases_when_republishing(self):
        self.publish()
        index_path = self.output / 'releases.json'
        index = json.loads(index_path.read_text())
        index.append({'id': 2, 'tag_name': 'v0.9.0', 'assets': []})
        index_path.write_text(json.dumps(index))
        self.publish()
        self.assertEqual([r['tag_name'] for r in json.loads(index_path.read_text())], ['v1.0.0', 'v0.9.0'])

    def test_missing_or_corrupt_binary_leaves_existing_index_unchanged(self):
        self.publish()
        original = (self.output / 'releases.json').read_bytes()
        binary = self.assets / 'c3_transponder-firmware.bin'
        binary.write_bytes(b'corrupt')
        with self.assertRaises(ValueError):
            self.publish()
        self.assertEqual((self.output / 'releases.json').read_bytes(), original)
        binary.unlink()
        with self.assertRaises((ValueError, FileNotFoundError)):
            self.publish()
        self.assertEqual((self.output / 'releases.json').read_bytes(), original)

    def test_rejects_traversal_and_nonstable_versions(self):
        for version in ['../v1.0.0', 'v1.0.0-rc1']:
            self.metadata['tag_name'] = version
            with self.assertRaises(ValueError):
                self.publish()

    def test_published_version_cannot_be_replaced_with_different_firmware(self):
        self.publish()
        original = (self.output / 'releases.json').read_bytes()
        binary = self.assets / 'c3_transponder-firmware.bin'
        binary.write_bytes(b'different but internally valid firmware')
        self.write_checksums()
        with self.assertRaisesRegex(ValueError, 'immutable'):
            self.publish()
        self.assertEqual((self.output / 'releases.json').read_bytes(), original)
        self.assertNotEqual((self.output / 'v1.0.0' / binary.name).read_bytes(), binary.read_bytes())


if __name__ == '__main__':
    unittest.main()
