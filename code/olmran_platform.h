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
    
#define ID_EDITCHILD         100
#define ID_INPUTCHILD        101
#define ID_CONTROLBACKGROUND 102
#define ID_CONTROLVITALS     103
#define ID_CONTROLPORTRAIT   104
#define ID_CONTROLPLAYER     105
#define ID_CONTROLCOMPASS    106
#define ID_CONTROLTIMER      107
    
#define IDC_MACRO_F1         2000
#define IDC_MACRO_F2         2001
#define IDC_MACRO_F3         2002
#define IDC_MACRO_F4         2003
#define IDC_MACRO_F5         2004
#define IDC_MACRO_F6         2005
#define IDC_MACRO_F7         2006
#define IDC_MACRO_F8         2007
#define IDC_MACRO_F9         2008
#define IDC_MACRO_F10        2009
#define IDC_MACRO_F11        2010
#define IDC_MACRO_F12        2011
#define IDC_MACRO_SHIFT_F1   2012
#define IDC_MACRO_SHIFT_F2   2013
#define IDC_MACRO_SHIFT_F3   2014
#define IDC_MACRO_SHIFT_F4   2015
#define IDC_MACRO_SHIFT_F5   2016
#define IDC_MACRO_SHIFT_F6   2017
#define IDC_MACRO_SHIFT_F7   2018
#define IDC_MACRO_SHIFT_F8   2019
#define IDC_MACRO_SHIFT_F9   2020
#define IDC_MACRO_SHIFT_F10  2021
#define IDC_MACRO_SHIFT_F11  2022
#define IDC_MACRO_SHIFT_F12  2023
    
#define IDC_MACRO_SAVE       2254
#define IDC_MACRO_CANCEL     2255
    
#define ID_ACTIONTIMER       255
    
#define IDM_FILE_QUIT 1
    
#define IDM_EDIT_ECHO    10
#define IDM_EDIT_PERSIST 11
    
#define IDM_MUSIC_ENABLED 20  
#define IDM_MUSIC_LOOP    21
#define IDM_MUSIC_SHUFFLE 22
    
#define IDM_MUSIC_DARK1   25
#define IDM_MUSIC_DARK2   26
#define IDM_MUSIC_DARK3   27
#define IDM_MUSIC_DARK4   28
    
#define IDM_MACRO_PLAYER 30
#define IDM_MACRO_GLOBAL 31
    
#define CONTROL_BITMAP "images/control.BMP"
    
#define DIR_SW  1
#define DIR_S  (1 << 1)
#define DIR_SE (1 << 2)
#define DIR_W  (1 << 3)
#define DIR_E  (1 << 4)
#define DIR_NW (1 << 5)
#define DIR_N  (1 << 6)
#define DIR_NE (1 << 7)
#define DIR_U  (1 << 8)
#define DIR_D  (1 << 9)
    
#define FLAG_LOGGEDIN 1
#define FLAG_ECHO    (1 << 1)
#define FLAG_PERSIST (1 << 2)
#define FLAG_MUSIC   (1 << 3)
#define FLAG_LOOP    (1 << 4)
#define FLAG_SHUFFLE (1 << 5)
    
    
    const uint8 SETTINGS_COUNT = 5;
    const uint8 MAX_MACROS = 24;
    
    char *MacroLabels[] = 
    {
        "F1",
        "F2",
        "F3",
        "F4",
        "F5",
        "F6",
        "F7",
        "F8",
        "F9",
        "F10",
        "F11",
        "F12",
        "Shift+F1",
        "Shift+F2",
        "Shift+F3",
        "Shift+F4",
        "Shift+F5",
        "Shift+F6",
        "Shift+F7",
        "Shift+F8",
        "Shift+F9",
        "Shift+F10",
        "Shift+F11",
        "Shift+F12",
    };
    
    uint16 MacroIDs[] = 
    {
        IDC_MACRO_F1,
        IDC_MACRO_F2,
        IDC_MACRO_F3,
        IDC_MACRO_F4,
        IDC_MACRO_F5,
        IDC_MACRO_F6,
        IDC_MACRO_F7,
        IDC_MACRO_F8,
        IDC_MACRO_F9,
        IDC_MACRO_F10,
        IDC_MACRO_F11,
        IDC_MACRO_F12,
        IDC_MACRO_SHIFT_F1,
        IDC_MACRO_SHIFT_F2,
        IDC_MACRO_SHIFT_F3,
        IDC_MACRO_SHIFT_F4,
        IDC_MACRO_SHIFT_F5,
        IDC_MACRO_SHIFT_F6,
        IDC_MACRO_SHIFT_F7,
        IDC_MACRO_SHIFT_F8,
        IDC_MACRO_SHIFT_F9,
        IDC_MACRO_SHIFT_F10,
        IDC_MACRO_SHIFT_F11,
        IDC_MACRO_SHIFT_F12,
    };
    
    
#if 1
    // Connect to local server
#define HOST_ADDRESS "192.168.1.208"
#else
    // Connect to olmran.net
#define HOST_ADDRESS "52.15.57.217"
#endif
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
