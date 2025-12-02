# NarratteMiru (傚って見る)

NaratteMiru is a tool to help debug Game Boy (Color) emulators.

## Features
- View/ format assembly
- View memory
- View Tilemaps/ Tiles/ Objects
- Control emulation speed
- Quick load different ROMs
- Take/ Load Snapshots
- Live reload emulator

## Usage
To make use of NaratteMiru, the emulator has to be able to be loaded as a library with **dlopen** and **dlsym**.

It also needs to implement some functions. 
Which functions are needed to be implemented depends on all requirements.
For example, if joypad input is not needed, *Input* does not have to be implemented.
The only necessary function to be implemented is *Tick*.

If the emulator's functions have different names then the default (which will probably be the case)
a file called symbols.conf can be created and placed in the same directory as the NaratteMiru executable.
The formatting is the following 
```
Init: my_init_func_name
Tick: my_tick_func_name
...
```

## Functions
### Init
```in8_t Init(void** cpu, void** mem, void** ppu, void** disassembler) ```

Initializes the emulator CPU, memory, PPU, and disassembler modules.
If NULL is passed to the disassembler the builtin disassembler will be used

### LoadRom
```int8_t LoadRom(void* mem, const char* boot, const char* game)``` 

Loads the boot ROM and game ROM into memory.

### Tick
```void Tick(void* cpu, void* mem, void* ppu)``` 

Executes a single emulation tick

### Input
```void Input(void* cpu, uint8_t inputMask)``` 

Feeds joypad input into the CPU.
Each bit represents a button state.

###GetInstructionCache
```void GetInstructionCache(void* cpu, uint8_t* ic)``` 

Gets the CPU’s last executed instruction. ic[0-2]: opcode, ic[3] opcode length

### Disassemble
```char* Disassemble(void* disassembler, uint8_t* instruction)``` 

Disassembles a single instruction byte sequence.
Returns a pointer to a human-readable string.

### Free
```void Free(void** cpu, void** mem, void** ppu, void** disassembler)``` 

Frees all emulator components allocated by Init.

### GetMemoryChange
```mem_change* GetMemoryChange(void* mem)``` 

Returns a linked list of memory changes since the last retrieval
(if memory-change tracking is enabled).
``` c
struct mem_change {
    struct mem_change* next;
    uint16_t addr;
    uint8_t data;
};
```

### EnableMemoryChange
```void EnableMemoryChange(void* mem, uint8_t enable)```

Enables (1) or disables (0) memory-change tracking.


### InitPseudoRam
```int8_t InitPseudoRam(void** mem, const char* boot, const char* game)```

Creates a new memory on which memory changes are applied without impacting the actual memory.
This is used in combination with *GetMemoryChange* to show the exact state of the memory at each instruction 
(only works correctly if monitoring is on since the start of emulation)

### FreePseudoRam
```void FreePseudoRam(void** mem)```

Frees pseudo-memory previously created by InitPseudoRam.


### MemoryRead
```uint8_t MemoryRead(void* mem, uint16_t addr)```

Reads a byte from emulated memory.


### MemoryWrite
```void MemoryWrite(void* mem, uint16_t addr, uint8_t data)```

Writes a byte to emulated memory.

### GetFrameBuffer
```uint32_t* GetFrameBuffer(void* ppu)```

Returns a pointer to the composed final framebuffer.

### GetFrameBuffers
```void GetFrameBuffers(void* ppu, uint32_t** bg, uint32_t** win, uint32_t** obj, uint32_t** prio)```

Returns pointers to the internal background, window, object, and priority buffers.
Buffers should return NULL if not used.

### GetSnapshot
```void GetSnapshot(const void* cpu, const void* mem, const void* ppu, uint8_t** buffer, uint32_t* size)```

Produces a full emulator snapshot.
The buffer must be allocated but not freed. 

### SetSnapshot

```void SetSnapshot(void* cpu, void* mem, void* ppu, const uint8_t* buffer, uint32_t size)```

Restores emulator state from a snapshot created by GetSnapshot.

# Notes
- While it *should* work on Windows/ macOS it has only been tested on Linux.  
- To build there is a high chance the CMake file has to be adjusted.
- While the content should be mostly accurate treat is with some caution, there might be some logic errors, especially with extreme edge cases

# Screenshots
<img width="2022" height="1224" alt="image" src="https://github.com/user-attachments/assets/c5e59c6c-ab76-4759-8822-e2b564d2de24" />
<img width="1412" height="879" alt="image" src="https://github.com/user-attachments/assets/69a56c9e-bc6f-4cf8-85c3-295e2ac87ca6" />
