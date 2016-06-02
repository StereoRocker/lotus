#include <stdint.h>
#include "common.h"
#include "dt.h"

/*								GDT shiz									  */
typedef struct {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

typedef struct {
	uint32_t prev_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

#define GDT_ENTS 6
gdt_entry_t gdt_ents[GDT_ENTS];
gdt_ptr_t	gdt_ptr;
tss_entry_t tss_entry;

extern void tss_flush();
extern void gdt_flush(uint32_t);

static void gdt_set_gate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
	gdt_ents[num].base_low 		= (base & 0xFFFF);
	gdt_ents[num].base_middle	= (base >> 16) & 0xFF;
	gdt_ents[num].base_high		= (base >> 24) & 0xFF;
	
	gdt_ents[num].limit_low 	= (limit & 0xFFFF);
	gdt_ents[num].granularity 	= (limit >> 16) & 0x0F;
	
	gdt_ents[num].granularity 	|= gran & 0xF0;
	gdt_ents[num].access		= access;
}

static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0)
{
	uint32_t base = (uint32_t)&tss_entry;
	uint32_t limit = base+sizeof(tss_entry);
	gdt_set_gate(num, base, limit, 0xE9, 0x00);
	
	memset((uint8_t*)&tss_entry, 0, sizeof(tss_entry));
	tss_entry.ss0 = ss0;
	tss_entry.esp0 = esp0;
	tss_entry.cs = 0x0B;
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}

static void init_gdt()
{
	gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_ENTS) - 1;
	gdt_ptr.base = (uint32_t)(&gdt_ents);
	
	gdt_set_gate(0, 0, 0, 0, 0);				// Null segment
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);	// Code segment
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);	// Data segment
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);	// User mode code segment
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);	// User mode data segment
	write_tss(5, 0x10, 0x0);					// TSS (allows syscalls)
	
	gdt_flush((uint32_t)(&gdt_ptr));
	tss_flush();
}

void set_kstack(uint32_t stack)
{
	tss_entry.esp0 = stack;
}



/*								IDT shiz									  */
typedef struct {
	uint16_t base_lo;
	uint16_t sel;
	uint8_t  always0;
	uint8_t  flags;
	uint16_t base_hi;
} __attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) idt_ptr_t;

extern void  isr0();
extern void  isr1();
extern void  isr2();
extern void  isr3();
extern void  isr4();
extern void  isr5();
extern void  isr6();
extern void  isr7();
extern void  isr8();
extern void  isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void int48();
extern void int49();
extern void int50();
extern void int51();
extern void int52();
extern void int53();
extern void int54();
extern void int55();
extern void int56();
extern void int57();
extern void int58();
extern void int59();
extern void int60();
extern void int61();
extern void int62();
extern void int63();
extern void int64();
extern void int65();
extern void int66();
extern void int67();
extern void int68();
extern void int69();
extern void int70();
extern void int71();
extern void int72();
extern void int73();
extern void int74();
extern void int75();
extern void int76();
extern void int77();
extern void int78();
extern void int79();
extern void int80();
extern void int81();
extern void int82();
extern void int83();
extern void int84();
extern void int85();
extern void int86();
extern void int87();
extern void int88();
extern void int89();
extern void int90();
extern void int91();
extern void int92();
extern void int93();
extern void int94();
extern void int95();
extern void int96();
extern void int97();
extern void int98();
extern void int99();
extern void int100();
extern void int101();
extern void int102();
extern void int103();
extern void int104();
extern void int105();
extern void int106();
extern void int107();
extern void int108();
extern void int109();
extern void int110();
extern void int111();
extern void int112();
extern void int113();
extern void int114();
extern void int115();
extern void int116();
extern void int117();
extern void int118();
extern void int119();
extern void int120();
extern void int121();
extern void int122();
extern void int123();
extern void int124();
extern void int125();
extern void int126();
extern void int127();
extern void int128();
extern void int129();
extern void int130();
extern void int131();
extern void int132();
extern void int133();
extern void int134();
extern void int135();
extern void int136();
extern void int137();
extern void int138();
extern void int139();
extern void int140();
extern void int141();
extern void int142();
extern void int143();
extern void int144();
extern void int145();
extern void int146();
extern void int147();
extern void int148();
extern void int149();
extern void int150();
extern void int151();
extern void int152();
extern void int153();
extern void int154();
extern void int155();
extern void int156();
extern void int157();
extern void int158();
extern void int159();
extern void int160();
extern void int161();
extern void int162();
extern void int163();
extern void int164();
extern void int165();
extern void int166();
extern void int167();
extern void int168();
extern void int169();
extern void int170();
extern void int171();
extern void int172();
extern void int173();
extern void int174();
extern void int175();
extern void int176();
extern void int177();
extern void int178();
extern void int179();
extern void int180();
extern void int181();
extern void int182();
extern void int183();
extern void int184();
extern void int185();
extern void int186();
extern void int187();
extern void int188();
extern void int189();
extern void int190();
extern void int191();
extern void int192();
extern void int193();
extern void int194();
extern void int195();
extern void int196();
extern void int197();
extern void int198();
extern void int199();
extern void int200();
extern void int201();
extern void int202();
extern void int203();
extern void int204();
extern void int205();
extern void int206();
extern void int207();
extern void int208();
extern void int209();
extern void int210();
extern void int211();
extern void int212();
extern void int213();
extern void int214();
extern void int215();
extern void int216();
extern void int217();
extern void int218();
extern void int219();
extern void int220();
extern void int221();
extern void int222();
extern void int223();
extern void int224();
extern void int225();
extern void int226();
extern void int227();
extern void int228();
extern void int229();
extern void int230();
extern void int231();
extern void int232();
extern void int233();
extern void int234();
extern void int235();
extern void int236();
extern void int237();
extern void int238();
extern void int239();
extern void int240();
extern void int241();
extern void int242();
extern void int243();
extern void int244();
extern void int245();
extern void int246();
extern void int247();
extern void int248();
extern void int249();
extern void int250();
extern void int251();
extern void int252();
extern void int253();
extern void int254();
extern void int255();
extern void idt_flush(uint32_t);

idt_entry_t idt_ents[256];
idt_ptr_t	idt_ptr;

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
	idt_ents[num].base_lo 	= base & 0xFFFF;
	idt_ents[num].base_hi 	= (base >> 16) & 0xFFFF;
	
	idt_ents[num].sel		= sel;
	idt_ents[num].always0 	= 0;
	idt_ents[num].flags		= flags | 0x60;	// This must be uncommented when we get to using user mode
}

static void init_idt()
{
	idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
	idt_ptr.base  = (uint32_t)(&idt_ents);
	
	memset((uint8_t*)(&idt_ents), 0, sizeof(idt_entry_t)*256);
	
	idt_set_gate( 0,  (uint32_t)isr0, 0x08, 0x8E);
	idt_set_gate( 1,  (uint32_t)isr1, 0x08, 0x8E);
	idt_set_gate( 2,  (uint32_t)isr2, 0x08, 0x8E);
	idt_set_gate( 3,  (uint32_t)isr3, 0x08, 0x8E);
	idt_set_gate( 4,  (uint32_t)isr4, 0x08, 0x8E);
	idt_set_gate( 5,  (uint32_t)isr5, 0x08, 0x8E);
	idt_set_gate( 6,  (uint32_t)isr6, 0x08, 0x8E);
	idt_set_gate( 7,  (uint32_t)isr7, 0x08, 0x8E);
	idt_set_gate( 8,  (uint32_t)isr8, 0x08, 0x8E);
	idt_set_gate( 9,  (uint32_t)isr9, 0x08, 0x8E);
	idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
	idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
	idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
	idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
	idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
	idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
	idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
	idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
	idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
	idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
	idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
	idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
	idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
	idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
	idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
	idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
	idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
	idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
	idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
	idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
	idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
	idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
	
	// Re-map the IRQ table
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);
	
	idt_set_gate(32,  (uint32_t)irq0, 0x08, 0x8E);
	idt_set_gate(33,  (uint32_t)irq1, 0x08, 0x8E);
	idt_set_gate(34,  (uint32_t)irq2, 0x08, 0x8E);
	idt_set_gate(35,  (uint32_t)irq3, 0x08, 0x8E);
	idt_set_gate(36,  (uint32_t)irq4, 0x08, 0x8E);
	idt_set_gate(37,  (uint32_t)irq5, 0x08, 0x8E);
	idt_set_gate(38,  (uint32_t)irq6, 0x08, 0x8E);
	idt_set_gate(39,  (uint32_t)irq7, 0x08, 0x8E);
	idt_set_gate(40,  (uint32_t)irq8, 0x08, 0x8E);
	idt_set_gate(41,  (uint32_t)irq9, 0x08, 0x8E);
	idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
	idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
	idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
	idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
	idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
	idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
	
	// Set up external interrupts
	    idt_set_gate(48, (uint32_t)int48, 0x08, 0x8E);
    idt_set_gate(49, (uint32_t)int49, 0x08, 0x8E);
    idt_set_gate(50, (uint32_t)int50, 0x08, 0x8E);
    idt_set_gate(51, (uint32_t)int51, 0x08, 0x8E);
    idt_set_gate(52, (uint32_t)int52, 0x08, 0x8E);
    idt_set_gate(53, (uint32_t)int53, 0x08, 0x8E);
    idt_set_gate(54, (uint32_t)int54, 0x08, 0x8E);
    idt_set_gate(55, (uint32_t)int55, 0x08, 0x8E);
    idt_set_gate(56, (uint32_t)int56, 0x08, 0x8E);
    idt_set_gate(57, (uint32_t)int57, 0x08, 0x8E);
    idt_set_gate(58, (uint32_t)int58, 0x08, 0x8E);
    idt_set_gate(59, (uint32_t)int59, 0x08, 0x8E);
    idt_set_gate(60, (uint32_t)int60, 0x08, 0x8E);
    idt_set_gate(61, (uint32_t)int61, 0x08, 0x8E);
    idt_set_gate(62, (uint32_t)int62, 0x08, 0x8E);
    idt_set_gate(63, (uint32_t)int63, 0x08, 0x8E);
    idt_set_gate(64, (uint32_t)int64, 0x08, 0x8E);
    idt_set_gate(65, (uint32_t)int65, 0x08, 0x8E);
    idt_set_gate(66, (uint32_t)int66, 0x08, 0x8E);
    idt_set_gate(67, (uint32_t)int67, 0x08, 0x8E);
    idt_set_gate(68, (uint32_t)int68, 0x08, 0x8E);
    idt_set_gate(69, (uint32_t)int69, 0x08, 0x8E);
    idt_set_gate(70, (uint32_t)int70, 0x08, 0x8E);
    idt_set_gate(71, (uint32_t)int71, 0x08, 0x8E);
    idt_set_gate(72, (uint32_t)int72, 0x08, 0x8E);
    idt_set_gate(73, (uint32_t)int73, 0x08, 0x8E);
    idt_set_gate(74, (uint32_t)int74, 0x08, 0x8E);
    idt_set_gate(75, (uint32_t)int75, 0x08, 0x8E);
    idt_set_gate(76, (uint32_t)int76, 0x08, 0x8E);
    idt_set_gate(77, (uint32_t)int77, 0x08, 0x8E);
    idt_set_gate(78, (uint32_t)int78, 0x08, 0x8E);
    idt_set_gate(79, (uint32_t)int79, 0x08, 0x8E);
    idt_set_gate(80, (uint32_t)int80, 0x08, 0x8E);
    idt_set_gate(81, (uint32_t)int81, 0x08, 0x8E);
    idt_set_gate(82, (uint32_t)int82, 0x08, 0x8E);
    idt_set_gate(83, (uint32_t)int83, 0x08, 0x8E);
    idt_set_gate(84, (uint32_t)int84, 0x08, 0x8E);
    idt_set_gate(85, (uint32_t)int85, 0x08, 0x8E);
    idt_set_gate(86, (uint32_t)int86, 0x08, 0x8E);
    idt_set_gate(87, (uint32_t)int87, 0x08, 0x8E);
    idt_set_gate(88, (uint32_t)int88, 0x08, 0x8E);
    idt_set_gate(89, (uint32_t)int89, 0x08, 0x8E);
    idt_set_gate(90, (uint32_t)int90, 0x08, 0x8E);
    idt_set_gate(91, (uint32_t)int91, 0x08, 0x8E);
    idt_set_gate(92, (uint32_t)int92, 0x08, 0x8E);
    idt_set_gate(93, (uint32_t)int93, 0x08, 0x8E);
    idt_set_gate(94, (uint32_t)int94, 0x08, 0x8E);
    idt_set_gate(95, (uint32_t)int95, 0x08, 0x8E);
    idt_set_gate(96, (uint32_t)int96, 0x08, 0x8E);
    idt_set_gate(97, (uint32_t)int97, 0x08, 0x8E);
    idt_set_gate(98, (uint32_t)int98, 0x08, 0x8E);
    idt_set_gate(99, (uint32_t)int99, 0x08, 0x8E);
    idt_set_gate(100, (uint32_t)int100, 0x08, 0x8E);
    idt_set_gate(101, (uint32_t)int101, 0x08, 0x8E);
    idt_set_gate(102, (uint32_t)int102, 0x08, 0x8E);
    idt_set_gate(103, (uint32_t)int103, 0x08, 0x8E);
    idt_set_gate(104, (uint32_t)int104, 0x08, 0x8E);
    idt_set_gate(105, (uint32_t)int105, 0x08, 0x8E);
    idt_set_gate(106, (uint32_t)int106, 0x08, 0x8E);
    idt_set_gate(107, (uint32_t)int107, 0x08, 0x8E);
    idt_set_gate(108, (uint32_t)int108, 0x08, 0x8E);
    idt_set_gate(109, (uint32_t)int109, 0x08, 0x8E);
    idt_set_gate(110, (uint32_t)int110, 0x08, 0x8E);
    idt_set_gate(111, (uint32_t)int111, 0x08, 0x8E);
    idt_set_gate(112, (uint32_t)int112, 0x08, 0x8E);
    idt_set_gate(113, (uint32_t)int113, 0x08, 0x8E);
    idt_set_gate(114, (uint32_t)int114, 0x08, 0x8E);
    idt_set_gate(115, (uint32_t)int115, 0x08, 0x8E);
    idt_set_gate(116, (uint32_t)int116, 0x08, 0x8E);
    idt_set_gate(117, (uint32_t)int117, 0x08, 0x8E);
    idt_set_gate(118, (uint32_t)int118, 0x08, 0x8E);
    idt_set_gate(119, (uint32_t)int119, 0x08, 0x8E);
    idt_set_gate(120, (uint32_t)int120, 0x08, 0x8E);
    idt_set_gate(121, (uint32_t)int121, 0x08, 0x8E);
    idt_set_gate(122, (uint32_t)int122, 0x08, 0x8E);
    idt_set_gate(123, (uint32_t)int123, 0x08, 0x8E);
    idt_set_gate(124, (uint32_t)int124, 0x08, 0x8E);
    idt_set_gate(125, (uint32_t)int125, 0x08, 0x8E);
    idt_set_gate(126, (uint32_t)int126, 0x08, 0x8E);
    idt_set_gate(127, (uint32_t)int127, 0x08, 0x8E);
    idt_set_gate(128, (uint32_t)int128, 0x08, 0x8E);
    idt_set_gate(129, (uint32_t)int129, 0x08, 0x8E);
    idt_set_gate(130, (uint32_t)int130, 0x08, 0x8E);
    idt_set_gate(131, (uint32_t)int131, 0x08, 0x8E);
    idt_set_gate(132, (uint32_t)int132, 0x08, 0x8E);
    idt_set_gate(133, (uint32_t)int133, 0x08, 0x8E);
    idt_set_gate(134, (uint32_t)int134, 0x08, 0x8E);
    idt_set_gate(135, (uint32_t)int135, 0x08, 0x8E);
    idt_set_gate(136, (uint32_t)int136, 0x08, 0x8E);
    idt_set_gate(137, (uint32_t)int137, 0x08, 0x8E);
    idt_set_gate(138, (uint32_t)int138, 0x08, 0x8E);
    idt_set_gate(139, (uint32_t)int139, 0x08, 0x8E);
    idt_set_gate(140, (uint32_t)int140, 0x08, 0x8E);
    idt_set_gate(141, (uint32_t)int141, 0x08, 0x8E);
    idt_set_gate(142, (uint32_t)int142, 0x08, 0x8E);
    idt_set_gate(143, (uint32_t)int143, 0x08, 0x8E);
    idt_set_gate(144, (uint32_t)int144, 0x08, 0x8E);
    idt_set_gate(145, (uint32_t)int145, 0x08, 0x8E);
    idt_set_gate(146, (uint32_t)int146, 0x08, 0x8E);
    idt_set_gate(147, (uint32_t)int147, 0x08, 0x8E);
    idt_set_gate(148, (uint32_t)int148, 0x08, 0x8E);
    idt_set_gate(149, (uint32_t)int149, 0x08, 0x8E);
    idt_set_gate(150, (uint32_t)int150, 0x08, 0x8E);
    idt_set_gate(151, (uint32_t)int151, 0x08, 0x8E);
    idt_set_gate(152, (uint32_t)int152, 0x08, 0x8E);
    idt_set_gate(153, (uint32_t)int153, 0x08, 0x8E);
    idt_set_gate(154, (uint32_t)int154, 0x08, 0x8E);
    idt_set_gate(155, (uint32_t)int155, 0x08, 0x8E);
    idt_set_gate(156, (uint32_t)int156, 0x08, 0x8E);
    idt_set_gate(157, (uint32_t)int157, 0x08, 0x8E);
    idt_set_gate(158, (uint32_t)int158, 0x08, 0x8E);
    idt_set_gate(159, (uint32_t)int159, 0x08, 0x8E);
    idt_set_gate(160, (uint32_t)int160, 0x08, 0x8E);
    idt_set_gate(161, (uint32_t)int161, 0x08, 0x8E);
    idt_set_gate(162, (uint32_t)int162, 0x08, 0x8E);
    idt_set_gate(163, (uint32_t)int163, 0x08, 0x8E);
    idt_set_gate(164, (uint32_t)int164, 0x08, 0x8E);
    idt_set_gate(165, (uint32_t)int165, 0x08, 0x8E);
    idt_set_gate(166, (uint32_t)int166, 0x08, 0x8E);
    idt_set_gate(167, (uint32_t)int167, 0x08, 0x8E);
    idt_set_gate(168, (uint32_t)int168, 0x08, 0x8E);
    idt_set_gate(169, (uint32_t)int169, 0x08, 0x8E);
    idt_set_gate(170, (uint32_t)int170, 0x08, 0x8E);
    idt_set_gate(171, (uint32_t)int171, 0x08, 0x8E);
    idt_set_gate(172, (uint32_t)int172, 0x08, 0x8E);
    idt_set_gate(173, (uint32_t)int173, 0x08, 0x8E);
    idt_set_gate(174, (uint32_t)int174, 0x08, 0x8E);
    idt_set_gate(175, (uint32_t)int175, 0x08, 0x8E);
    idt_set_gate(176, (uint32_t)int176, 0x08, 0x8E);
    idt_set_gate(177, (uint32_t)int177, 0x08, 0x8E);
    idt_set_gate(178, (uint32_t)int178, 0x08, 0x8E);
    idt_set_gate(179, (uint32_t)int179, 0x08, 0x8E);
    idt_set_gate(180, (uint32_t)int180, 0x08, 0x8E);
    idt_set_gate(181, (uint32_t)int181, 0x08, 0x8E);
    idt_set_gate(182, (uint32_t)int182, 0x08, 0x8E);
    idt_set_gate(183, (uint32_t)int183, 0x08, 0x8E);
    idt_set_gate(184, (uint32_t)int184, 0x08, 0x8E);
    idt_set_gate(185, (uint32_t)int185, 0x08, 0x8E);
    idt_set_gate(186, (uint32_t)int186, 0x08, 0x8E);
    idt_set_gate(187, (uint32_t)int187, 0x08, 0x8E);
    idt_set_gate(188, (uint32_t)int188, 0x08, 0x8E);
    idt_set_gate(189, (uint32_t)int189, 0x08, 0x8E);
    idt_set_gate(190, (uint32_t)int190, 0x08, 0x8E);
    idt_set_gate(191, (uint32_t)int191, 0x08, 0x8E);
    idt_set_gate(192, (uint32_t)int192, 0x08, 0x8E);
    idt_set_gate(193, (uint32_t)int193, 0x08, 0x8E);
    idt_set_gate(194, (uint32_t)int194, 0x08, 0x8E);
    idt_set_gate(195, (uint32_t)int195, 0x08, 0x8E);
    idt_set_gate(196, (uint32_t)int196, 0x08, 0x8E);
    idt_set_gate(197, (uint32_t)int197, 0x08, 0x8E);
    idt_set_gate(198, (uint32_t)int198, 0x08, 0x8E);
    idt_set_gate(199, (uint32_t)int199, 0x08, 0x8E);
    idt_set_gate(200, (uint32_t)int200, 0x08, 0x8E);
    idt_set_gate(201, (uint32_t)int201, 0x08, 0x8E);
    idt_set_gate(202, (uint32_t)int202, 0x08, 0x8E);
    idt_set_gate(203, (uint32_t)int203, 0x08, 0x8E);
    idt_set_gate(204, (uint32_t)int204, 0x08, 0x8E);
    idt_set_gate(205, (uint32_t)int205, 0x08, 0x8E);
    idt_set_gate(206, (uint32_t)int206, 0x08, 0x8E);
    idt_set_gate(207, (uint32_t)int207, 0x08, 0x8E);
    idt_set_gate(208, (uint32_t)int208, 0x08, 0x8E);
    idt_set_gate(209, (uint32_t)int209, 0x08, 0x8E);
    idt_set_gate(210, (uint32_t)int210, 0x08, 0x8E);
    idt_set_gate(211, (uint32_t)int211, 0x08, 0x8E);
    idt_set_gate(212, (uint32_t)int212, 0x08, 0x8E);
    idt_set_gate(213, (uint32_t)int213, 0x08, 0x8E);
    idt_set_gate(214, (uint32_t)int214, 0x08, 0x8E);
    idt_set_gate(215, (uint32_t)int215, 0x08, 0x8E);
    idt_set_gate(216, (uint32_t)int216, 0x08, 0x8E);
    idt_set_gate(217, (uint32_t)int217, 0x08, 0x8E);
    idt_set_gate(218, (uint32_t)int218, 0x08, 0x8E);
    idt_set_gate(219, (uint32_t)int219, 0x08, 0x8E);
    idt_set_gate(220, (uint32_t)int220, 0x08, 0x8E);
    idt_set_gate(221, (uint32_t)int221, 0x08, 0x8E);
    idt_set_gate(222, (uint32_t)int222, 0x08, 0x8E);
    idt_set_gate(223, (uint32_t)int223, 0x08, 0x8E);
    idt_set_gate(224, (uint32_t)int224, 0x08, 0x8E);
    idt_set_gate(225, (uint32_t)int225, 0x08, 0x8E);
    idt_set_gate(226, (uint32_t)int226, 0x08, 0x8E);
    idt_set_gate(227, (uint32_t)int227, 0x08, 0x8E);
    idt_set_gate(228, (uint32_t)int228, 0x08, 0x8E);
    idt_set_gate(229, (uint32_t)int229, 0x08, 0x8E);
    idt_set_gate(230, (uint32_t)int230, 0x08, 0x8E);
    idt_set_gate(231, (uint32_t)int231, 0x08, 0x8E);
    idt_set_gate(232, (uint32_t)int232, 0x08, 0x8E);
    idt_set_gate(233, (uint32_t)int233, 0x08, 0x8E);
    idt_set_gate(234, (uint32_t)int234, 0x08, 0x8E);
    idt_set_gate(235, (uint32_t)int235, 0x08, 0x8E);
    idt_set_gate(236, (uint32_t)int236, 0x08, 0x8E);
    idt_set_gate(237, (uint32_t)int237, 0x08, 0x8E);
    idt_set_gate(238, (uint32_t)int238, 0x08, 0x8E);
    idt_set_gate(239, (uint32_t)int239, 0x08, 0x8E);
    idt_set_gate(240, (uint32_t)int240, 0x08, 0x8E);
    idt_set_gate(241, (uint32_t)int241, 0x08, 0x8E);
    idt_set_gate(242, (uint32_t)int242, 0x08, 0x8E);
    idt_set_gate(243, (uint32_t)int243, 0x08, 0x8E);
    idt_set_gate(244, (uint32_t)int244, 0x08, 0x8E);
    idt_set_gate(245, (uint32_t)int245, 0x08, 0x8E);
    idt_set_gate(246, (uint32_t)int246, 0x08, 0x8E);
    idt_set_gate(247, (uint32_t)int247, 0x08, 0x8E);
    idt_set_gate(248, (uint32_t)int248, 0x08, 0x8E);
    idt_set_gate(249, (uint32_t)int249, 0x08, 0x8E);
    idt_set_gate(250, (uint32_t)int250, 0x08, 0x8E);
    idt_set_gate(251, (uint32_t)int251, 0x08, 0x8E);
    idt_set_gate(252, (uint32_t)int252, 0x08, 0x8E);
    idt_set_gate(253, (uint32_t)int253, 0x08, 0x8E);
    idt_set_gate(254, (uint32_t)int254, 0x08, 0x8E);
    idt_set_gate(255, (uint32_t)int255, 0x08, 0x8E);
    
	idt_flush((uint32_t)(&idt_ptr));
	
	// Make sure we start interrupts once we're prepared to deal with them
	asm volatile("sti");
}

// Set up the descriptor tables
void dt_init()
{
	init_gdt();
	init_idt();
}
