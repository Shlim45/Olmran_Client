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
#define ID_CHATCHILD         102
    
#define ID_CONTROLBACKGROUND 200
#define ID_CONTROLVITALS     201
#define ID_CONTROLPORTRAIT   202
#define ID_CONTROLPLAYER     203
#define ID_CONTROLCOMPASS    204
#define ID_CONTROLTIMER      205
    
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
#define IDC_MACRO_CTRL_A     2024
#define IDC_MACRO_CTRL_B     2025
#define IDC_MACRO_CTRL_D     2026
#define IDC_MACRO_CTRL_E     2027
#define IDC_MACRO_CTRL_F     2028
#define IDC_MACRO_CTRL_G     2029
#define IDC_MACRO_CTRL_H     2030
#define IDC_MACRO_CTRL_I     2031
#define IDC_MACRO_CTRL_J     2032
#define IDC_MACRO_CTRL_K     2033
#define IDC_MACRO_CTRL_L     2034
#define IDC_MACRO_CTRL_M     2035
#define IDC_MACRO_CTRL_N     2036
#define IDC_MACRO_CTRL_O     2037
#define IDC_MACRO_CTRL_P     2038
#define IDC_MACRO_CTRL_Q     2039
#define IDC_MACRO_CTRL_R     2040
#define IDC_MACRO_CTRL_S     2041
#define IDC_MACRO_CTRL_T     2042
#define IDC_MACRO_CTRL_U     2043
#define IDC_MACRO_CTRL_W     2044
#define IDC_MACRO_CTRL_X     2045
#define IDC_MACRO_CTRL_Y     2046
#define IDC_MACRO_CTRL_Z     2047
#define IDC_MACRO_SHIFT_CTRL_A     2048
#define IDC_MACRO_SHIFT_CTRL_B     2049
#define IDC_MACRO_SHIFT_CTRL_D     2050
#define IDC_MACRO_SHIFT_CTRL_E     2051
#define IDC_MACRO_SHIFT_CTRL_F     2052
#define IDC_MACRO_SHIFT_CTRL_G     2053
#define IDC_MACRO_SHIFT_CTRL_H     2054
#define IDC_MACRO_SHIFT_CTRL_I     2055
#define IDC_MACRO_SHIFT_CTRL_J     2056
#define IDC_MACRO_SHIFT_CTRL_K     2057
#define IDC_MACRO_SHIFT_CTRL_L     2058
#define IDC_MACRO_SHIFT_CTRL_M     2059
#define IDC_MACRO_SHIFT_CTRL_N     2060
#define IDC_MACRO_SHIFT_CTRL_O     2061
#define IDC_MACRO_SHIFT_CTRL_P     2062
#define IDC_MACRO_SHIFT_CTRL_Q     2063
#define IDC_MACRO_SHIFT_CTRL_R     2064
#define IDC_MACRO_SHIFT_CTRL_S     2065
#define IDC_MACRO_SHIFT_CTRL_T     2066
#define IDC_MACRO_SHIFT_CTRL_U     2067
#define IDC_MACRO_SHIFT_CTRL_W     2068
#define IDC_MACRO_SHIFT_CTRL_X     2069
#define IDC_MACRO_SHIFT_CTRL_Y     2070
#define IDC_MACRO_SHIFT_CTRL_Z     2071
    
#define IDC_MACRO_SAVE       2254
#define IDC_MACRO_CANCEL     2255
    
#define ID_ACTIONTIMER       255
    
#define IDM_FILE_QUIT 1
    
#define IDM_EDIT_ECHO    10
#define IDM_EDIT_PERSIST 11
#define IDM_EDIT_CHAT    12
    
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
#define FLAG_CHAT    (1 << 6)
    
    
    const uint8 SETTINGS_COUNT = 5;
    const uint8 MAX_MACROS = 72;
    
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
        "Ctrl+A",
        "Ctrl+B",
        "Ctrl+D",
        "Ctrl+E",
        "Ctrl+F",
        "Ctrl+G",
        "Ctrl+H",
        "Ctrl+I",
        "Ctrl+J",
        "Ctrl+K",
        "Ctrl+L",
        "Ctrl+M",
        "Ctrl+N",
        "Ctrl+O",
        "Ctrl+P",
        "Ctrl+Q",
        "Ctrl+R",
        "Ctrl+S",
        "Ctrl+T",
        "Ctrl+U",
        "Ctrl+W",
        "Ctrl+X",
        "Ctrl+Y",
        "Ctrl+Z",
        "Shift+Ctrl+A",
        "Shift+Ctrl+B",
        "Shift+Ctrl+D",
        "Shift+Ctrl+E",
        "Shift+Ctrl+F",
        "Shift+Ctrl+G",
        "Shift+Ctrl+H",
        "Shift+Ctrl+I",
        "Shift+Ctrl+J",
        "Shift+Ctrl+K",
        "Shift+Ctrl+L",
        "Shift+Ctrl+M",
        "Shift+Ctrl+N",
        "Shift+Ctrl+O",
        "Shift+Ctrl+P",
        "Shift+Ctrl+Q",
        "Shift+Ctrl+R",
        "Shift+Ctrl+S",
        "Shift+Ctrl+T",
        "Shift+Ctrl+U",
        "Shift+Ctrl+W",
        "Shift+Ctrl+X",
        "Shift+Ctrl+Y",
        "Shift+Ctrl+Z",
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
        IDC_MACRO_CTRL_A,
        IDC_MACRO_CTRL_B,
        IDC_MACRO_CTRL_D,
        IDC_MACRO_CTRL_E,
        IDC_MACRO_CTRL_F,
        IDC_MACRO_CTRL_G,
        IDC_MACRO_CTRL_H,
        IDC_MACRO_CTRL_I,
        IDC_MACRO_CTRL_J,
        IDC_MACRO_CTRL_K,
        IDC_MACRO_CTRL_L,
        IDC_MACRO_CTRL_M,
        IDC_MACRO_CTRL_N,
        IDC_MACRO_CTRL_O,
        IDC_MACRO_CTRL_P,
        IDC_MACRO_CTRL_Q,
        IDC_MACRO_CTRL_R,
        IDC_MACRO_CTRL_S,
        IDC_MACRO_CTRL_T,
        IDC_MACRO_CTRL_U,
        IDC_MACRO_CTRL_W,
        IDC_MACRO_CTRL_X,
        IDC_MACRO_CTRL_Y,
        IDC_MACRO_CTRL_Z,
        IDC_MACRO_SHIFT_CTRL_A,
        IDC_MACRO_SHIFT_CTRL_B,
        IDC_MACRO_SHIFT_CTRL_D,
        IDC_MACRO_SHIFT_CTRL_E,
        IDC_MACRO_SHIFT_CTRL_F,
        IDC_MACRO_SHIFT_CTRL_G,
        IDC_MACRO_SHIFT_CTRL_H,
        IDC_MACRO_SHIFT_CTRL_I,
        IDC_MACRO_SHIFT_CTRL_J,
        IDC_MACRO_SHIFT_CTRL_K,
        IDC_MACRO_SHIFT_CTRL_L,
        IDC_MACRO_SHIFT_CTRL_M,
        IDC_MACRO_SHIFT_CTRL_N,
        IDC_MACRO_SHIFT_CTRL_O,
        IDC_MACRO_SHIFT_CTRL_P,
        IDC_MACRO_SHIFT_CTRL_Q,
        IDC_MACRO_SHIFT_CTRL_R,
        IDC_MACRO_SHIFT_CTRL_S,
        IDC_MACRO_SHIFT_CTRL_T,
        IDC_MACRO_SHIFT_CTRL_U,
        IDC_MACRO_SHIFT_CTRL_W,
        IDC_MACRO_SHIFT_CTRL_X,
        IDC_MACRO_SHIFT_CTRL_Y,
        IDC_MACRO_SHIFT_CTRL_Z,
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
