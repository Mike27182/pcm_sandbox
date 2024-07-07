#include <cstdint>
#include <vector>

#pragma once

struct PmcConfig
{
    uint64_t eventSelect;
    uint64_t umask;
    uint64_t edgeDetect;
    uint64_t anyThread;
    uint64_t invert;
    uint64_t cmask;
    uint64_t pebs3F6H;
    uint64_t pebs3F7H;
    uint64_t offcore;
    const char* name;
};

uint64_t read_msr_f( int cpu, uint32_t msr );
void write_msr_f( int cpu, uint32_t msr, uint64_t value );
void enable_counters( uint32_t cpu );
void setPmc( uint32_t cpu, uint32_t pmc, PmcConfig cfg );
void pinToCore( uint32_t cpu );
void disable_apic( int cpu );

[[gnu::always_inline]] inline uint64_t readPmc( uint32_t counter )
{
    uint32_t low, high;
    asm volatile( "rdpmc" : "=a"( low ), "=d"( high ) : "c"( counter ) );
    return ( ( uint64_t )high << 32 ) | low;
}

[[gnu::always_inline]] inline uint64_t readFixPmc( uint32_t counter )
{
    /*
    https://perfmon-events.intel.com/
    */

    constexpr uint64_t IA32_FIXED_CTR0_INDEX = 0x40000000;

    return readPmc( IA32_FIXED_CTR0_INDEX + counter );
}

[[gnu::always_inline]] inline uint64_t rdtscp( uint32_t& cpu )
{
    uint32_t eax, edx;
    asm volatile (
        "rdtscp"
        : "=a" ( eax ), "=d" ( edx ), "=c" ( cpu )
    );
    return ( ( uint64_t )edx << 32 ) | eax;
}

[[gnu::always_inline]] inline uint64_t rdtsc()
{
    uint32_t eax, edx;
    asm volatile (
        "rdtsc"
        : "=a" ( eax ), "=d" ( edx )
    );
    return ( ( uint64_t )edx << 32 ) | eax;
}

extern std::vector< PmcConfig > pmcConfigsRaptorLakePCore;
extern std::vector< PmcConfig > pmcConfigsRaptorLakeECore;
extern std::vector< PmcConfig > pmcConfigsRaptorLakeUNCore;
extern std::vector< PmcConfig > pmcConfigsRaptorLakePCoreOffcore;
extern std::vector< PmcConfig > pmcConfigsRaptorLakeECoreOffcore;
extern std::vector< PmcConfig > pmcConfigsCoffeeLake;
extern std::vector< const char* > pmcFixNotes;

