/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jonathan Hawranko $
   $Notice: (C) Copyright 2020 by Jonathan Hawranko. All Rights Reserved. $
   ======================================================================== */
#ifndef OLMRAN_PLATFORM_H
#define OLMRAN_PLATFORM_H


/*
 * NOTE(jon):
 * 
 * OLMRAN_INTERNAL:
 *  0 - Build for public release
 *  1 - Build for developer only
 * 
 * OLMRAN_SLOW:
 *  0 - No slow code allowed!
 *  1 - Slow code welcome.
 */

#ifdef __cplusplus
extern "C" {
#endif
    
    //
    // NOTE(jon): Compilers
    //
#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif
    
#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif
    
#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
    // TODO(jon): MOAR COMPILERZ!!!
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif
    
#if COMPILER_MSVC
#include <intrin.h>
#endif
    
    //
    // NOTE(jon): Types
    //
    
#include <stdint.h>
#include <stddef.h>
    
    typedef int8_t int8;
    typedef int16_t int16;
    typedef int32_t int32;
    typedef int64_t int64;
    typedef int32 bool32;
    
    typedef uint8_t uint8;
    typedef uint16_t uint16;
    typedef uint32_t uint32;
    typedef uint64_t uint64;
    
    typedef size_t memory_index;
    
    typedef float real32;
    typedef double real64;
    
#define internal static
#define local_persist static
#define global_variable static
    
#define ID_EDITCHILD 100
#define HOST_ADDRESS "192.168.1.208"
#define HOST_PORT 4000
    
#if OLMRAN_SLOW
#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif
    
    // TODO(jon): Should these always be 64-bit?
#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)
    
#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))
    // TODO(jon): swamp, min, max ... macros???
    
    inline uint32 SafeTruncateUInt64(uint64 Value)
    {
        // TODO(jon): Defines for maximum values UInt32Max
        Assert(Value <= 0xFFFFFFFF);
        uint32 Result = (uint32)Value;
        return Result;
    }
    
    typedef struct thread_context
    {
        int Placeholder;
    } thread_context;
    
    /*
      NOTE(jon): Services that the platform layer provides to the game.
    */
#if OLMRAN_INTERNAL
    /* IMPORTANT(jon):
    
        These are NOT for doing anything in the shipping game - they are
        blocking and the write doesn't protect against lost data!
    
    */
    typedef struct debug_read_file_result
    {
        uint32 ContentsSize;
        void *Contents;
    } debug_read_file_result;
    
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *Thread, void *Memory)
    typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);
    
#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *Thread, char *Filename)
    typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);
    
#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(thread_context *Thread, char *Filename, uint32 MemorySize, void *Memory)
    typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
    
#endif
    
    typedef struct game_sound_output_buffer
    {
        int SamplesPerSecond;
        int SampleCount;
        int16 *Samples;
    } game_sound_output_buffer;
    
    typedef struct game_memory
    {
        bool32 IsInitialized;
        
        uint64 PermanentStorageSize;
        void *PermanentStorage; // NOTE(jon): REQUIRED to be cleared to zero at startup
        
        uint64 TransientStorageSize;
        void *TransientStorage; // NOTE(jon): REQUIRED to be cleared to zero at startup
        
        //debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
        //debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
        //debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
    } game_memory;
    
#ifdef __cplusplus
}
#endif

#endif //OLMRAN_PLATFORM_H