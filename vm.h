#ifndef INSITUCS_VM_HEADER
#define INSITUCS_VM_HEADER

typedef enum {
  CMP, INCOND,                 //
  JMP_REG, JMP_IM, JMP_MEM,    //
  CALL_REG, CALL_IM, CALL_MEM, // Control-flow
  RET,                         //

  MOV_REG_MEM, MOV_IM_MEM, MOV_MEM_MEM, MOV_MEM_REG, MOV_IM_REG, //
  PUSH_STACK,                                                    // Memory handling
  STO_LONGS,                                                     // 

  OFFS_REG, // Basic control-registers operations

  ADDD_MEM_MEM, ADDD_MEM_REG, ADDD_REG_REG, //
  SUBD_MEM_MEM, SUBD_MEM_REG, SUBD_REG_REG, //
  MULD_MEM_MEM, MULD_MEM_REG, MULD_REG_REG, // Numeric operations
  DIVD_MEM_MEM, DIVD_MEM_REG, DIVD_REG_REG, //
// shifts?

  AND_MEM_MEM, AND_MEM_REG, AND_REG_REG, //
  OR_MEM_MEM , OR_MEM_REG , OR_REG_REG,  //
  XOR_MEM_MEM, XOR_MEM_REG, XOR_REG_REG, // Boolean operations
  NOT_MEM, NOT_REG,                      //

  // ------------------------- Complex instruction set ------------------------- //

  MAP,    //
  FILTER, // List-manipulating instructions
  REDUCE, //

  LAMBDA, // Creation of lambdas

} Bytecode;

/*** How the VM works:
 *
 ** Basic structure

 * We are given 2 pointers, and we will create another one
 * like this:
 *
 * ----------------------------------------------
 * |    .data     |    .funcs     |    .main    |
 * |--------------|---------------|-------------|
 * |    long      |  Instruction  | Instruction |
 * |    long      |  Instruction  | Instruction |
 * |    long      |  Instruction  | Instruction |
 * |    long      |  Instruction  |             |
 * |              |  Instruction  |             |
 * |              |               |             |
 * ----------------------------------------------
 * // (We separate functions and main so its easier for the compiler to gather all the 
 * // things that need to happen first)
 *
 * The .data section serves as out stack, the .funcs section contains all the functions/lambdas
 * we may use/return throught the program, and the .main section is the entry point for our VM
 *
 * The VM will also have another array that holds all of the pointers returned by the STO_LONGS
 * instruction so we are able to clean them after the stack frame of wherever it was created 
 * collapses
 *
 ** Operation of the VM
 *
 * Control flow:
 *
 * Jumps operate by changing the VM's program counter, which is a pointer to the next instruction
 * to be executed
 *
 * We will hold a value (that serves as register) containing the result of cmp operations,
 * and whenever a jump instruction occurs we will this result
 *
 * Calls do several things (may not occur in the following order):
 * - They set the program counter to the begining of the called function
 * - They set the called function's stack frame...
 *   - Setting the stack base pointer to the stack pointer
 *   - Setting the stack pointer to be the offset provided to it, this offset is calculated by
 *     the compiler by comparing the number of arguments that it is passed to the number of arguments
 *     asked and providing whichever is larger
 * - They set the call stack to point to the next instruction in the calling function
 * - And lastly, they pass the required arguments and nulling arguments not provided if necessary
 *
 * ---------------
 * |    .data    |
 * |-------------|
 * |    long     | <- Stack base pointer |
 * |    ....     |                       |
 * |    long     |                       |
 * | - - - - - - | <- Stack pointer | <--- New stack base pointer
 * |    arg      |                  |
 * |    arg      |                  |
 * |    arg      |                  |
 * |    ...      | <----------------- New stack pointer
 * |-------------|
 *
 * --------------- ------------------
 * |    .funcs   | | Program counter|
 * |-------------| ------------------
 * |     ins     |  |
 * |     call    | -------->|
 * |     ins     | < - - - -|<- -
 * |     ret     |          |   |
 * |             |          |   |
 * |     ins     | <--------|   |
 * |     ...     | ...          |
 * |     ins     |              |
 * |     ret     | - - - - - - >|
 * |             |
 * |-------------|
 *
 * Memory managment:
 *
 * Everything stored (allocated) while on a function will be put onto the cleaning stack, so when the called function
 * returns it cleans everything that isnt returned (and that isnt referenced inside the returned pointer), so basically
 * a garbage collector
 *
 * Actual operation:
 *
 * The VM will read the instruction stream and execute the instruction (they hopefully have clear enough names). Do note
 * that register operation will be less refined that what the instruction set (at least for now) allows
 *
 */

typedef struct ExecutionRes {
  bool is_ok;
  char* err;
} ExecutionRes;

ExecutionRes exec(Bytecode* main, Bytecode* funcs);

#endif  // INSITUCS_VM_HEADER
