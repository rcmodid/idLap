#if defined(IDLAP_PROFIL_TRANSPONDER)
#include "../firmware/transponder/src/main.cpp"
#elif defined(IDLAP_PROFIL_DECODER)
#include "../firmware/decoder/src/main.cpp"
#else
#error "Pilih profil firmware idLap melalui PlatformIO"
#endif
