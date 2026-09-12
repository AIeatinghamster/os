#include "idt.c"
extern void keyboard_stub(void);
typedef struct String String;
struct String{
    char text[160];
    int size;
};
String join(String s1, String s2){
    String tmp;
    tmp.size=0;
    for (int i; s1.text[i]=="\0"; i++){
        if (sizeof(tmp.text) >= 160)
            return tmp;
        tmp.text[tmp.size++] = s1.text[i];
    }
    for (int i; s2.text[i]=="\0"; i++){
        if (sizeof(tmp.text) >= 160)
            return tmp;
        tmp.text[tmp.size++] = s2.text[i];
    }
    return tmp;
}
int id_counter = 0;
char *shell_text_mem_buffer=(char *)0xB8000;
int line_text_buffer=0;
const char *next_line_offset=160;

int cursor=0;
void new_line(){
    
    shell_text_mem_buffer+=160;
    cursor=0;
}
void print_char(char c){
    char *video = (char *)shell_text_mem_buffer;

    video[cursor * 2] = c;
    video[cursor * 2 + 1] = 0x07;
    cursor++;
    if (cursor==80){
        cursor=0;
        new_line();
    }
}
void print(char to_print[], int next_line) {
    char *video = (char *)shell_text_mem_buffer;
    int ln=0;
    for (int i = 0; to_print[i] != '\0'; i++) {
        print_char(to_print[i]);
    }
    if (next_line==1){
        new_line();
    }
}

#define CAPACITY 100

typedef struct Folder Folder;
typedef struct Array Array;
struct Folder {
    char name[100];
    int id;
    Array *subfolders;
};
struct Array {
    Folder data[CAPACITY];
    int size;
};



char current_path_str[] = ".";

void append(Array *arr, Folder value) {
    if (arr->size >= CAPACITY)
        return;

    arr->data[arr->size++] = value;
}

void add_subfolder(Folder *fld, char name[]) {
    Folder to_add;

    int i = 0;
    while (name[i] != '\0' && i < 99) {
        to_add.name[i] = name[i];
        i++;
    }
    to_add.name[i] = '\0';

    id_counter++;
    to_add.id = id_counter;

    append(fld->subfolders, to_add);
}

void print_subfolders(Folder *fld) {
    Array *subf = fld->subfolders;

    for (int i = 0; i < subf->size; i++) {
        print(subf->data[i].name, 1);
    }
}

Folder current_folder;

void proces_cmd(char cmd[], char parms[]) {
    if (cmd[0] == 'p' &&
        cmd[1] == 'w' &&
        cmd[2] == 'd' &&
        cmd[3] == '\0') {

        print(current_path_str, 0);
    }
    else if (cmd[0] == 'l' &&
             cmd[1] == 's' &&
             cmd[2] == '\0') {

        print_subfolders(&current_folder);
    }
}
//input
unsigned char inb(unsigned short port)
{
    unsigned char result;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(result)
        : "Nd"(port)
    );

    return result;
}
void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}
void pic_remap(void)
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);
}
char scancode_to_char(unsigned char scancode)
{
    switch (scancode) {
        case 0x1E: return 'a';
        case 0x30: return 'b';
        case 0x2E: return 'c';
        case 0x20: return 'd';
        case 0x12: return 'e';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x17: return 'i';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';
        case 0x32: return 'm';
        case 0x31: return 'n';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x10: return 'q';
        case 0x13: return 'r';
        case 0x1F: return 's';
        case 0x14: return 't';
        case 0x16: return 'u';
        case 0x2F: return 'v';
        case 0x11: return 'w';
        case 0x2D: return 'x';
        case 0x15: return 'y';
        case 0x2C: return 'z';
        case 0x39: return ' ';

        default: return 0;
    }
}

void keyboard_handler(void){
    unsigned char scancode=inb(0x60);
    if (scancode==0x1C){
        new_line();
        outb(0x20, 0x20);
    }
    char c=scancode_to_char(scancode);

    if (c!=0){
        print_char(c);
    }
    outb(0x20, 0x20);
}
//input

void load_idt(void)
{
    struct idt_ptr idtp;

    idtp.limit = sizeof(idt) - 1;
    idtp.base = (unsigned int)&idt;

    __asm__ volatile (
        "lidtl (%0)"
        :
        : "r" (&idtp)
    );
}
unsigned short read_cs(void)
{
    unsigned short cs;

    __asm__ volatile (
        "mov %%cs, %0"
        : "=r"(cs)
    );

    return cs;
}
void kernel_main(void) {
    unsigned short cs;

    __asm__ volatile (
        "mov %%cs, %0"
        : "=r"(cs)
    );

    idt_set_gate(
        33,
        (unsigned int)keyboard_stub,
        cs,
        0x8E
    );

    load_idt();
    pic_remap();

    __asm__ volatile ("sti");
    Folder root;
    add_subfolder(&root, "test");
    add_subfolder(&root, "test2");
    //current_folder=root;
    print_subfolders(&root);
    while (1) {
    }
}
