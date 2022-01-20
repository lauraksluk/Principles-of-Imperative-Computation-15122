#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "lib/xalloc.h"
#include "lib/stack.h"
#include "lib/contracts.h"
#include "lib/c0v_stack.h"
#include "lib/c0vm.h"
#include "lib/c0vm_c0ffi.h"
#include "lib/c0vm_abort.h"

/* call stack frames */
typedef struct frame_info frame;
struct frame_info {
  c0v_stack_t S; /* Operand stack of C0 values */
  ubyte *P;      /* Function body */
  size_t pc;     /* Program counter */
  c0_value *V;   /* The local variables */
};

// Helper function to push integers onto the operand stack.
void push_int(c0v_stack_t S, int32_t i) {
  c0v_push(S, int2val(i));
  return;
}

// Helper function to pop integers from the operand stack.
int32_t pop_int(c0v_stack_t S) {
  return val2int(c0v_pop(S));
}

// Main execution function.
int execute(struct bc0_file *bc0) {
  REQUIRES(bc0 != NULL);

  // Operand stack of C0 values.
  c0v_stack_t S = c0v_stack_new();

  // Array of bytes that make up the current function.
  // Execution always starts with the main function (first in array).
  ubyte *P = bc0->function_pool[0].code;

  // Current location within the current byte array P. 
  size_t pc = 0;

  // Initialize local variables array.
  uint16_t vars = bc0->function_pool[0].num_vars;
  c0_value *V = xcalloc(vars, sizeof(c0_value));
  ASSERT(V != NULL);

  // The call stack, a generic stack that should contain pointers to frames.
  gstack_t callStack = stack_new();

  while (true) {
/*
#ifdef DEBUG
    
    fprintf(stderr, "Opcode %x -- Stack size: %zu -- PC: %zu\n",
            P[pc], c0v_stack_size(S), pc);
#endif
*/
    switch (P[pc]) {

    /* Additional stack operation: */

    case POP: {
      pc++;
      c0v_pop(S);
      break;
    }

    case DUP: {
      pc++;
      c0_value v = c0v_pop(S);
      c0v_push(S,v);
      c0v_push(S,v);
      break;
    }

    case SWAP: {
      pc++;
      c0_value v2 = c0v_pop(S);
      c0_value v1 = c0v_pop(S);
      c0v_push(S, v2);
      c0v_push(S, v1);
      break;
    }

    // Returning from a function.
    case RETURN: {
/*      
#ifdef DEBUG
      fprintf(stderr, "Returning %d from execute()\n", retval);
#endif
*/
      // Free local variable array.
      free(V);

      if (stack_empty(callStack)) {

        // Obtain return value from operand stack.
        c0_value val = c0v_pop(S);
        assert(c0v_stack_empty(S));
        int retval = val2int(val);

        // Free operand and call stack.
        c0v_stack_free(S);
        stack_free(callStack, NULL);

        // Return excecuted function value.
        return retval;
      
      } else {

        // Obtain return value from operand stack.
        c0_value val = c0v_pop(S);
        // Free current operand stack.
        c0v_stack_free(S);

        // Pop top frame off and restore variables
        frame *current = (frame*)pop(callStack);
        ASSERT(current != NULL);
        S = current->S;
        P = current->P;
        pc = current->pc;
        V = current->V;

        // Free frame and push returned value onto operand stack.
        free(current);
        c0v_push(S, val);

        break;
      }
    }

    /* Arithmetic and Logical operations */

    // Addition arithmetic operation.
    case IADD: {
      pc++;
      int val2 = pop_int(S);
      int val1 = pop_int(S);
      int sum = val1 + val2;
      push_int(S, sum);
      break;
    }

    // Subtraction arithmetic operation.
    case ISUB: {
      pc++;
      int val4 = pop_int(S);
      int val3 = pop_int(S);
      int diff = val3 - val4;
      push_int(S, diff);
      break;
    }

    // Multiplication arithmetic operation.
    case IMUL: {
      pc++;
      int val6 = pop_int(S);
      int val5 = pop_int(S);
      int product = val5 * val6;
      push_int(S, product);
      break;
    }

    // Division arithmetic operation.
    case IDIV: {
      pc++;
      int val8 = pop_int(S);
      if (val8 == 0) c0_arith_error("division by 0.");
      int val7 = pop_int(S);
      if (val8 == -1 && val7 == -2147483648) {
        c0_arith_error("division by 0.");
      }
      int div = val7 / val8;
      push_int(S, div);
      break;
    }

    // Modulo arithmetic operation.
    case IREM: {
      pc++;
      int val10 = pop_int(S);
      if (val10 == 0) c0_arith_error("division by 0.");
      int val9 = pop_int(S);
      if (val9 == -2147483648 && val10 == -1) {
        c0_arith_error("division by 0.");
      }
      int rem = val9 % val10;
      push_int(S, rem);
      break;
    }

    // Logical AND operation.
    case IAND: {
      pc++;
      int y1 = pop_int(S);
      int x1 = pop_int(S);
      int and = x1 & y1;
      push_int(S, and);
      break;
    }

    // Logical OR operation.
    case IOR: {
      pc++;
      int y2 = pop_int(S);
      int x2 = pop_int(S);
      int or = x2 | y2;
      push_int(S, or);
      break;
    }

    // Logical XOR (exclusive or) operation.
    case IXOR: {
      pc++;
      int y3 = pop_int(S);
      int x3 = pop_int(S);
      int xor = x3 ^ y3;
      push_int(S, xor);
      break;
    }

    // Bit-shifting: left shift operation.
    case ISHL: {
      pc++;
      int y4 = pop_int(S);
      if (0 <= y4 && y4 < 32) {
        int x4 = pop_int(S);
        int ls = x4 << y4;
        push_int(S, ls);
      } else {
        c0_arith_error("division by 0.");
      }
      break;
    }

    // Bit-shifting: right shift operation.
    case ISHR: {
      pc++;
      int y5 = pop_int(S);
      if (0 <= y5 && y5 < 32) {
        int x5 = pop_int(S);
        int rs = x5 >> y5;
        push_int(S, rs);
      } else {
        c0_arith_error("division by 0.");
      }
      break;
    }

    /* Pushing constants */

    // Integer constant (from instruction operand) loading instruction.
    case BIPUSH: {
      pc += 2;
      int32_t num = (int32_t)(byte)P[pc - 1];
      push_int(S, num);
      break;
    }

    // Integer constant loading instruction (from integer pool).
    case ILDC: {
      pc += 3;
      uint32_t indexI = (((uint32_t)P[pc - 2]) << 8) | ((uint32_t)P[pc - 1]);
      int32_t const1 = bc0->int_pool[indexI];
      push_int(S, const1);
      break;
    }

    // String constant loading instruction (from string pool).
    case ALDC: {
      pc += 3;
      uint16_t indexS = (((uint16_t)P[pc - 2]) << 8) | ((uint16_t)P[pc - 1]);
      char* const2 = &(bc0->string_pool[indexS]);
      c0v_push(S, ptr2val((void*)const2));
      break;
    }

    // NULL constant loading instruction.
    case ACONST_NULL: {
      pc++;
      c0_value nul = ptr2val((void*)0);
      c0v_push(S, nul);
      break;
    }

    /* Operations on local variables */

    // Load local variable from V onto operand stack.
    case VLOAD: {
      pc += 2;
      c0_value load = V[P[pc - 1]];
      c0v_push(S, load);
      break;
    }

    // Store local variable from operand stack in V.
    case VSTORE: {
      pc += 2;
      c0_value store = c0v_pop(S);
      V[P[pc - 1]] = store;
      break;
    }

    /* Assertions and errors */

    // Implements C0 built-in error() function.
    case ATHROW: {
      pc++;
      c0_value errV = c0v_pop(S);
      char* errmsg = (char*)val2ptr(errV);
      c0_user_error(errmsg);
      break;
    }

    // Implements C0 contracts and assertions.
    case ASSERT: {
      pc++;
      c0_value err = c0v_pop(S);
      int x = pop_int(S);
      if (x == 0) {
        char* msg = (char*)val2ptr(err);
        c0_assertion_failure(msg);
      }
      break;
    }

    /* Control flow operations */
    /* Implements conditional instructions for PC jumps (loops). */

    // Instruction has no effect.
    case NOP: {
      pc++;
      break;
    }

    // PC increments by offset if values are equal.
    case IF_CMPEQ: {
      c0_value v2 = c0v_pop(S);
      c0_value v1 = c0v_pop(S);
      if (val_equal(v1, v2)) {
        int16_t o1 = (int16_t)(uint16_t)P[pc + 1];
        int16_t o2 = (int16_t)(uint16_t)P[pc + 2];
        int16_t off = (int16_t)((o1 << 8) | o2);
        pc += off;
      } else {
        pc += 3;
      }
      break;
    }

    // PC increments by offset if values are not equal.
    case IF_CMPNE: {
      c0_value v2 = c0v_pop(S);
      c0_value v1 = c0v_pop(S);
      if (!val_equal(v1, v2)) {
        int16_t o1 = (int16_t)(uint16_t)P[pc + 1];
        int16_t o2 = (int16_t)(uint16_t)P[pc + 2];
        int16_t off = (int16_t)((o1 << 8) | o2);
        pc += off;
      } else {
        pc += 3;
      }
      break;
    }

    // PC increments by offset if x is less than y.
    case IF_ICMPLT: {
      int y1 = pop_int(S);
      int x1 = pop_int(S);
      if (x1 < y1) {
        int16_t o1 = (int16_t)(uint16_t)P[pc + 1];
        int16_t o2 = (int16_t)(uint16_t)P[pc + 2];
        int16_t off = (int16_t)((o1 << 8) | o2);
        pc += off;
      } else {
        pc += 3;
      }
      break;
    }

    // PC increments by offset if x is greater than/equal to y.
    case IF_ICMPGE: {
      int y2 = pop_int(S);
      int x2 = pop_int(S);
      if (x2 >= y2) {
        int16_t o1 = (int16_t)(uint16_t)P[pc + 1];
        int16_t o2 = (int16_t)(uint16_t)P[pc + 2];
        int16_t off = (int16_t)((o1 << 8) | o2);
        pc += off;
      } else {
        pc += 3;
      }
      break;
    }

    // PC increments by offset if x is greater than y.
    case IF_ICMPGT: {
      int y3 = pop_int(S);
      int x3 = pop_int(S);
      if (x3 > y3) {
        int16_t o1 = (int16_t)(uint16_t)P[pc + 1];
        int16_t o2 = (int16_t)(uint16_t)P[pc + 2];
        int16_t off = (int16_t)((o1 << 8) | o2);
        pc += off;
      } else {
        pc += 3;
      }
      break;
    }

    // PC increments by offset if x is less than/equal to y.
    case IF_ICMPLE: {
      int y4 = pop_int(S);
      int x4 = pop_int(S);
      if (x4 <= y4) {
        int16_t o1 = (int16_t)(uint16_t)P[pc + 1];
        int16_t o2 = (int16_t)(uint16_t)P[pc + 2];
        int16_t off = (int16_t)((o1 << 8) | o2);
        pc += off;
      } else {
        pc += 3;
      }
      break;
    }

    // PC increments by offset.
    case GOTO: {
      int16_t o1 = (int16_t)(uint16_t)P[pc + 1];
      int16_t o2 = (int16_t)(uint16_t)P[pc + 2];
      int16_t off = (int16_t)((o1 << 8) | o2);
      pc += off;
      break;
    }

    /* Function call operations: */

    // Implements local function calls.
    case INVOKESTATIC: {

      // Update PC and obtain bytes for function pool index.
      pc += 3;
      uint16_t c1 = (uint16_t)P[pc - 2];
      uint16_t c2 = (uint16_t)P[pc - 1];

      // Initialize frame and push onto callStack.
      frame *f = xmalloc(sizeof(frame));
      ASSERT(f != NULL);
      f->S = S;
      f->P = P;
      f->pc = pc;
      f->V = V;
      push(callStack, (void*)f);

      // Reset PC and function body pointer to new function.
      pc = 0;
      uint16_t index = (uint16_t)(c1 << 8) | c2;
      P = bc0->function_pool[index].code;

      // Initialize new variable array to arguments on old operand stack.
      uint16_t num_new_vars = bc0->function_pool[index].num_vars;
      V = xcalloc(num_new_vars, sizeof(c0_value));
      ASSERT(V != NULL);

      // Copy over arguments from old operand stack in opposite order.
      uint16_t num_args = bc0->function_pool[index].num_args;
      while (num_args != (uint16_t)0) {
        V[num_args - 1] = c0v_pop(S);
        num_args--;
      }

      // Initialize new operand stack.
      S = c0v_stack_new();

      break;
    }

    // Implements C0/C library function calls.
    case INVOKENATIVE: {

      // Update PC and obtain bytes for function pool index.
      pc += 3;
      uint16_t c1 = (uint16_t)P[pc - 2];
      uint16_t c2 = (uint16_t)P[pc - 1];

      // Initialize new variable array to arguments on old operand stack.
      uint16_t pool_index = (uint16_t)(c1 << 8) | c2;
      uint16_t new_args = bc0->native_pool[pool_index].num_args;
      c0_value *temp = xcalloc(new_args, sizeof(c0_value));
      ASSERT(temp != NULL);

      // Copy over arguments from old operand stack in opposite order.
      while (new_args != (uint16_t)0) {
        temp[new_args - 1] = c0v_pop(S);
        new_args--;
      }

      // Call library function with new variable array.
      uint16_t index = bc0->native_pool[pool_index].function_table_index;
      c0_value val = (native_function_table[index])(temp);

      // Free temporary variable array.
      free(temp);

      // Push result back onto operand stack.
      c0v_push(S, val);

      break;
    }


    /* Memory allocation operations: */

    // Implements allocating pointers and structs.
    case NEW: {
      pc += 2;
      uint8_t size = P[pc - 1];
      void *new = xcalloc(1, size);
      ASSERT(new != NULL);
      c0v_push(S, ptr2val(new));
      break;
    }

    // Implements allocating arrays.
    case NEWARRAY: {
      pc += 2;
      uint8_t size = P[pc - 1];
      int num = pop_int(S);
      if (num < 0) {
        c0_memory_error("invalid array size.");
      }
      c0_array *new = xcalloc(1, sizeof(c0_array));
      ASSERT(new != NULL);
      new->count = num;
      new->elt_size = (int32_t)(int8_t)size;
      new->elems = xcalloc(num, size);
      ASSERT(new->elems != NULL);
      c0v_push(S, ptr2val(new));
      break;
    }

    // Obtains the length of array on operand stack.
    case ARRAYLENGTH: {
      pc++;
      c0_array *arr = (c0_array*)val2ptr(c0v_pop(S));
      if (arr == NULL) {
        c0_memory_error("null pointer was accessed.");
      }
      int len = arr->count;
      push_int(S, len);
      break;
    }

    /* Memory access operations: */

    // Address arithmetic offset computation for structs.
    // Implements field accesses.
    case AADDF: {
      pc += 2;
      uint8_t field = P[pc - 1];
      unsigned char *a = (unsigned char*)val2ptr(c0v_pop(S));
      if (a == NULL) {
        c0_memory_error("null pointer was accessed.");
      }
      c0v_push(S, ptr2val(a + field));
      break;
    }

    // Implements array accesses.
    case AADDS: {
      pc++;
      int32_t index = pop_int(S);
      c0_array *array = (c0_array*)val2ptr(c0v_pop(S));
      if (array == NULL) {
        c0_memory_error("null pointer was accessed.");
      }
      int32_t count = array->count;
      if (!(0 <= index && index < count)) {
        c0_memory_error("invalid index access.");
      }
      int32_t size = array->elt_size;
      unsigned char *arr = (unsigned char*)array->elems;
      void *address = &arr[size*index];
      c0v_push(S, ptr2val(address));
      break;
    }

    // Implements reading from memory (integers).
    case IMLOAD: {
      pc++;
      int32_t *ipoint = (int32_t*)val2ptr(c0v_pop(S));
      if (ipoint == NULL) {
        c0_memory_error("null pointer was accessed.");
      }
      int32_t val = *ipoint;
      push_int(S, val);
      break;
    }

    // Implements writing to memory (integers).
    case IMSTORE: {
      pc++;
      int32_t val = pop_int(S);
      int32_t *intpoint = (int32_t*)val2ptr(c0v_pop(S));
      if (intpoint == NULL) {
        c0_memory_error("null pointer was accessed.");
      }
      *intpoint = val;
      break;
    }

    // Implements reading from memory (pointers).
    case AMLOAD: {
      pc++;
      void **point = val2ptr(c0v_pop(S));
      if (point == NULL) {
        c0_memory_error("null pointer was accessed.");
      }
      void *b = *point;
      c0v_push(S, ptr2val(b));
      break;
    }

    // Implements writing to memory (pointers).
    case AMSTORE: {
      pc++;
      void *b = val2ptr(c0v_pop(S));
      void **a = val2ptr(c0v_pop(S));
      if (a == NULL) {
        c0_memory_error("null pointer was accessed.");
      }
      *a = b;
      break;
    }

    // Implements reading from memory (characters).
    case CMLOAD: {
      pc++;
      unsigned char *cpoint = (unsigned char*)val2ptr(c0v_pop(S));
      if (cpoint == NULL) {
        c0_memory_error("null pointer was accessed.");
      }
      int32_t val = (int32_t)(int8_t)(*cpoint);
      push_int(S, val);
      break;
    }

    // Implements writing to memory (characters).
    case CMSTORE: {
      pc++;
      int32_t val = pop_int(S);
      unsigned char *charpoint = (unsigned char*)val2ptr(c0v_pop(S));
      if (charpoint == NULL) {
        c0_memory_error("null pointer was accessed.");
      }
      *charpoint = (unsigned char)(val & 0x7f);
      break;
    }

    // Invalid opcode.
    default:
      fprintf(stderr, "invalid opcode: 0x%02x\n", P[pc]);
      abort();
    }
  }

  /* cannot get here from infinite loop */
  assert(false);
}
