

void * memset ( void * ptr, int value, size_t num )
{
   uint64_t i=0;

   // find out if "ptr" is aligned on a SSE_XMM_SIZE boundary
   if((uint64_t)ptr & (SSE_XMM_SIZE - 1))
   {
      // we need to set byte-by-byte until "ptr" is aligned on an SSE_XMM_SIZE boundary
      // ... and lets make sure we don't copy 'too' many bytes (i < num)
      for(; ((uint64_t)ptr + i) & (SSE_XMM_SIZE - 1) && i < num; i++)
         asm("stosb;" :: "D"((uint64_t)ptr + i), "a"((unsigned char)value));
   }

   uint32_t sse_val[4];
   sse_val[0] = value << 8*0 | value << 8*1 | value << 8*2 | value << 8*3;
   sse_val[1] = value << 8*0 | value << 8*1 | value << 8*2 | value << 8*3;
   sse_val[2] = value << 8*0 | value << 8*1 | value << 8*2 | value << 8*3;
   sse_val[3] = value << 8*0 | value << 8*1 | value << 8*2 | value << 8*3;

   // set 64-byte chunks of memory (4 16-byte operations)
   for(; i + 64 <= num; i += 64)
   {
      asm volatile("movdqu (%1), %%xmm0;"    // set XMM0 to value
                   "movdqa %%xmm0, 0(%0);"   // move 16 bytes from XMM0 to %0 + 0
                   "movdqa %%xmm0, 16(%0);"
                   "movdqa %%xmm0, 32(%0);"
                   "movdqa %%xmm0, 48(%0);"
                   :: "r"((uint64_t)ptr + i), "r"(sse_val));
   }

   // copy the remaining bytes (if any)
   asm(" rep stosb; " :: "a"((unsigned char)value), "D"(((uint64_t)ptr) + i), "c"(num - i));

   return ptr;
}
