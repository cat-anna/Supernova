#ifndef _INLINES_H_
#define _INLINES_H_

static inline void BitSet(unsigned *v, unsigned bit)
{
	asm("bts %1,%0" : "+r" (*v) : "g"(bit));
}

static inline void BitClear(unsigned *v, unsigned bit)
{
	asm("btr %1,%0" : "+r" (*v) : "g"(bit));
}

static inline unsigned BitSearchLSB(unsigned v)
{
	unsigned ret;
	asm volatile ("bsf %1,%0" : "=a" (ret) : "d"(v));
	return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
	asm volatile("outb %0,%1"::"a"(val), "Nd" (port));
}

static inline unsigned char inb(unsigned short port) {
	unsigned char ret;
	asm volatile ("inb %1,%0":"=a"(ret):"Nd"(port));
	return ret;
}

static inline unsigned short inw(unsigned short port) {
	unsigned short ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

#ifndef _SUPERNOVA_H

inline static void outw(int port, unsigned short data) {
	__asm__ __volatile__("outw %%ax,%%dx\n\t" :: "a" (data), "d" (port));
}

#endif

inline static void repinsw(int port, void* va, int count) {
	__asm__ __volatile__ ("rep\n\t"
			"insw" :: "d" (port), "D" (va), "c" (count));
}

inline static void repoutsw(int port, void* va, int count) {
	__asm__ __volatile__ ("rep\n\t"
			"outsw" :: "d" (port), "D" (va), "c" (count));
}

inline static void fast_copy(void* src, void* dest, int count) {
	__asm__ __volatile__ ("rep\n\t"
			"movsl"
			:: "S" (src), "D" (dest), "c" (count));
}

inline static void ldcr4(unsigned long val) {
	__asm__ __volatile__ ("movl %0, %%cr4\n\t" : : "r" (val));
}

inline static unsigned long get_cr4(void) {
	register unsigned long res;

	__asm__ __volatile__( "movl %%cr4, %0\n\t" : "=r" (res) :);
	return (res);
}

inline static void ldcr3(unsigned long val) {
	__asm__ __volatile__ ("movl %0, %%cr3\n\t" : : "r" (val));
}

inline static unsigned long get_cr3(void) {
	register unsigned long res;

	__asm__ __volatile__( "movl %%cr3, %0\n\t" : "=r" (res) :);
	return (res);
}

inline static void ldcr0(unsigned long val) {
	__asm__ __volatile__ ("movl %0, %%cr0\n\t" : : "r" (val));
}

inline static unsigned long get_cr0(void) {
	register unsigned long res;

	__asm__ __volatile__( "movl %%cr0, %0\n\t" : "=r" (res) :);
	return (res);
}

inline static unsigned long get_esp() {
	unsigned long __ret;
	__asm__ __volatile__("movl %%esp, %0":"=r"(__ret));
	return __ret;
}

inline static void ldesp(unsigned long val) {
	__asm__ __volatile__ ("movl %0, %%esp\n\t" : : "r" (val));
}

static inline unsigned short get_ss() {
	unsigned short __ret;
	__asm__ __volatile__("mov %%ss,%w0":"=r"(__ret));
	return __ret;
}

static inline unsigned short get_gs() {
	unsigned short __ret;
	__asm__ __volatile__("mov %%gs,%w0":"=r"(__ret));
	return __ret;
}

static inline unsigned short get_fs() {
	unsigned short __ret;
	__asm__ __volatile__("mov %%fs,%w0":"=r"(__ret));
	return __ret;
}

static inline unsigned short get_es() {
	unsigned short __ret;
	__asm__ __volatile__("mov %%es,%w0":"=r"(__ret));
	return __ret;
}

static inline unsigned short get_cs() {
	unsigned short __ret;
	__asm__ __volatile__("mov %%cs,%w0":"=r"(__ret));
	return __ret;
}

static inline unsigned short get_ds() {
	unsigned short __ret;
	__asm__ __volatile__("mov %%ds,%w0":"=r"(__ret));
	return __ret;
}

inline static unsigned long get_eax() {
	unsigned long __ret;
	__asm__ __volatile__("movl %%eax,%0":"=r"(__ret));
	return __ret;
}

inline static void ldeax(unsigned long val) {
	__asm__ __volatile__ ("movl %0, %%eax\n\t" : : "r" (val));
}

inline static unsigned long get_ebx() {
	unsigned long __ret;
	__asm__ __volatile__("movl %%ebx,%0":"=r"(__ret));
	return __ret;
}

inline static unsigned long get_ecx() {
	unsigned long __ret;
	__asm__ __volatile__("movl %%ecx,%0":"=r"(__ret));
	return __ret;
}

inline static unsigned long get_edx() {
	unsigned long __ret;
	__asm__ __volatile__("movl %%edx,%0":"=r"(__ret));
	return __ret;
}

inline static unsigned long get_ebp() {
	unsigned long __ret;
	__asm__ __volatile__("movl %%ebx,%0":"=r"(__ret));
	return __ret;
}

inline static unsigned long get_esi() {
	unsigned long __ret;
	__asm__ __volatile__("movl %%esi,%0":"=r"(__ret));
	return __ret;
}

inline static unsigned long get_edi() {
	unsigned long __ret;
	__asm__ __volatile__("movl %%edi,%0":"=r"(__ret));
	return __ret;
}

static inline void FlushOneTlb(void* m) {
	asm volatile("invlpg (%0)" : :"r"(m));
}

#ifndef _SUPERNOVA_H

static inline void Breakpoint() {
	outw(0x8A00, 0x8A00);
	outw(0x8A00, 0x8aE0);
}

#endif


static inline void cpuid(unsigned code, unsigned *a, unsigned *d) {
  asm volatile("cpuid":"=a"(*a),"=d"(*d):"0"(code):"ecx","ebx");
}

/** issue a complete request, storing general registers output as a string
 */
static inline int cpuid_string(unsigned code, unsigned where[4]) {
	unsigned highest;
  asm volatile("cpuid":"=a"(*where),"=b"(*(where+1)),
               "=c"(*(where+2)),"=d"(*(where+3)):"0"(code));
  return highest;
}

#endif
