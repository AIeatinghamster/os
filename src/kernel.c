#include "idt.c"
extern void keyboard_stub(void);
void *memcpy(void *dest, const void *src, unsigned int n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

    for (unsigned int i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}
void *memset(void *dest, int value, unsigned int n)
{
    unsigned char *d = dest;

    for (unsigned int i = 0; i < n; i++) {
        d[i] = (unsigned char)value;
    }

    return dest;
}
typedef struct String String;
struct String{
    char text[160];
    int size;
};

String char_to_str(char c)
{
    String tmp;

    tmp.text[0] = c;
    tmp.text[1] = '\0';
    tmp.size = 1;

    return tmp;
}
String join(String s1, String s2)
{
    String tmp;
    tmp.size = 0;

    int i = 0;

    while (i < s1.size && tmp.size < 159) {
        tmp.text[tmp.size++] = s1.text[i++];
    }

    i = 0;

    while (i < s2.size && tmp.size < 159) {
        tmp.text[tmp.size++] = s2.text[i++];
    }

    tmp.text[tmp.size] = '\0';

    return tmp;
}
void clear(String *s)
{
    for (int i = 0; i < 160; i++)
        s->text[i] = '\0';

    s->size = 0;
}



int id_counter = 0;
char *shell_text_mem_buffer=(char *)0xB8000;
int line_text_buffer=0;
const char *next_line_offset=160;
String line = {
    .text = {0},
    .size = 0
};

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
    for (int i = 0; to_print[i] != '\0'; i++) {
        print_char(to_print[i]);
    }
    if (next_line==1){
        new_line();
    }
}
void type(char c){
    print_char(c);
    line = join(line, char_to_str(c));
}

#define CAPACITY 100

typedef struct Folder Folder;
typedef struct Array Array;
typedef struct cmd cmd;
typedef struct cmdArray cmdArray;
typedef struct strArray strArray; 


struct Folder {
    char name[100];
    int id;
    Array *subfolders;
    int sub_chid;
    Folder *parent;
};
struct Array {
    Folder data[CAPACITY];
    int size;
};
void append(Array *arr, Folder value) {
    if (arr->size >= CAPACITY)
        return;

    arr->data[arr->size++] = value;
}


char current_path_str[] = ".";
Folder *current_folder;
Folder *previous_current_folder;

Array folder_subfolders[CAPACITY];
void add_subfolder(Folder *fld, char name[]) {
    Folder to_add;

    int i = 0;
    while (name[i] != '\0' && i < 99) {
        to_add.name[i] = name[i];
        i++;
    }
    to_add.name[i] = '\0';
    to_add.parent = fld;
    id_counter++;
    to_add.id = id_counter;
    fld->sub_chid++;
    to_add.sub_chid=0;
    to_add.subfolders = &folder_subfolders[to_add.id];
    append(fld->subfolders, to_add);
}

void print_subfolders(Folder *fld) {
    Array *subf = fld->subfolders;

    for (int i = 0; i < subf->size; i++) {
        print(subf->data[i].name, 1);
    }
}
struct strArray {
    String data[CAPACITY];
    int size;
};
void append_str(strArray *arr, String value) {
    if (arr->size >= CAPACITY)
        return;

    arr->data[arr->size++] = value;
}
struct cmd{
    strArray parms;
    String command;
};
struct cmdArray {
    cmd data[CAPACITY];
    int size;
};
void append_cmd(cmdArray *arr, cmd value) {
    if (arr->size >= CAPACITY)
        return;

    arr->data[arr->size++] = value;
}
/*void remove_front_cmd(cmdArray *arr){
    if (arr->size==0) return;
    for (int i=0; arr->data[i]!='\0'; i++){
        if (i==0) continue;
        arr->data[i]=arr->data[i-1];
    }
    arr->data[arr->size-1]='\0';
    arr->size-=1;
}*/


cmdArray command_queue={0};
int comp_str(char c1[], char c2[])
{
    for (int i = 0; i < 100; i++) {
        if (c1[i] != c2[i])
            return 0;

        if (c1[i] == '\0')
            return 1;
    }

    return 0;
}
void process_cmd(cmd cmd) {
    String cmdt=cmd.command;
    if (cmdt.text[0] == 'p' &&
        cmdt.text[1] == 'w' &&
        cmdt.text[2] == 'd' &&
        cmdt.text[3] == '\0') {
        print(current_path_str, 1);
    }
    else if (cmdt.text[0] == 'l' &&
             cmdt.text[1] == 's' &&
             cmdt.text[2] == '\0') {
        if (current_folder->sub_chid!=0) print_subfolders(current_folder);
        else print("empty", 1);
    }
    else if (cmdt.text[0] == 'c' &&
            cmdt.text[1] == 'd' &&
            cmdt.text[2] == '\0') {

        if (cmd.parms.size == 0) {
            print("cd: missing argument", 1);
            return;
        }
        if (current_folder->parent != 0) {
            current_folder = current_folder->parent;
        }

        for (int i = 0; i < current_folder->subfolders->size; i++) {

            if (comp_str(
                    current_folder->subfolders->data[i].name,
                    cmd.parms.data[0].text
                )) {
                previous_current_folder=current_folder;
                current_folder = &current_folder->subfolders->data[i];
                return;
            }
        }

        print("cd: folder not found", 1);
    }
    else if (cmdt.text[0] == 'm' && cmdt.text[1] == 'k' && cmdt.text[2] == 'd' && cmdt.text[3] == 'i' && cmdt.text[4] == 'r'){
        add_subfolder(current_folder, cmd.parms.data[0].text);
    }
}


void extract_parms(String raw_cmd)
{
    cmd tmp_cmd = {0};

    int i = 0;

    while (raw_cmd.text[i] == ' ')
        i++;

    while (raw_cmd.text[i] != '\0' &&
           raw_cmd.text[i] != ' ' &&
           tmp_cmd.command.size < 159)
    {
        tmp_cmd.command.text[tmp_cmd.command.size++] = raw_cmd.text[i++];
    }

    tmp_cmd.command.text[tmp_cmd.command.size] = '\0';

    while (raw_cmd.text[i] == ' ')
        i++;

    while (raw_cmd.text[i] != '\0')
    {
        String param = {0};

        while (raw_cmd.text[i] != '\0' &&
               raw_cmd.text[i] != ' ' &&
               param.size < 159)
        {
            param.text[param.size++] = raw_cmd.text[i++];
        }

        param.text[param.size] = '\0';

        if (param.size > 0)
            append_str(&tmp_cmd.parms, param);

        while (raw_cmd.text[i] == ' ')
            i++;
    }

    append_cmd(&command_queue, tmp_cmd);
    process_cmd(tmp_cmd);
}


//input start
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
int shift_pressed=0;

char scancode_to_char(unsigned char scancode)
{
    switch (scancode) {
        case 0x2A:
        case 0x36:
            shift_pressed = 1;
            return 0;

        case 0xAA:
        case 0xB6:
            shift_pressed = 0;
            return 0;


        case 0x0B: return shift_pressed ? ')' : '0';
        case 0x02: return shift_pressed ? '!' : '1';
        case 0x03: return shift_pressed ? '@' : '2';
        case 0x04: return shift_pressed ? '#' : '3';
        case 0x05: return shift_pressed ? '$' : '4';
        case 0x06: return shift_pressed ? '%' : '5';
        case 0x07: return shift_pressed ? '^' : '6';
        case 0x08: return shift_pressed ? '&' : '7';
        case 0x09: return shift_pressed ? '*' : '8';
        case 0x0A: return shift_pressed ? '(' : '9';


        case 0x1E: return shift_pressed ? 'A' : 'a';
        case 0x30: return shift_pressed ? 'B' : 'b';
        case 0x2E: return shift_pressed ? 'C' : 'c';
        case 0x20: return shift_pressed ? 'D' : 'd';
        case 0x12: return shift_pressed ? 'E' : 'e';
        case 0x21: return shift_pressed ? 'F' : 'f';
        case 0x22: return shift_pressed ? 'G' : 'g';
        case 0x23: return shift_pressed ? 'H' : 'h';
        case 0x17: return shift_pressed ? 'I' : 'i';
        case 0x24: return shift_pressed ? 'J' : 'j';
        case 0x25: return shift_pressed ? 'K' : 'k';
        case 0x26: return shift_pressed ? 'L' : 'l';
        case 0x32: return shift_pressed ? 'M' : 'm';
        case 0x31: return shift_pressed ? 'N' : 'n';
        case 0x18: return shift_pressed ? 'O' : 'o';
        case 0x19: return shift_pressed ? 'P' : 'p';
        case 0x10: return shift_pressed ? 'Q' : 'q';
        case 0x13: return shift_pressed ? 'R' : 'r';
        case 0x1F: return shift_pressed ? 'S' : 's';
        case 0x14: return shift_pressed ? 'T' : 't';
        case 0x16: return shift_pressed ? 'U' : 'u';
        case 0x2F: return shift_pressed ? 'V' : 'v';
        case 0x11: return shift_pressed ? 'W' : 'w';
        case 0x2D: return shift_pressed ? 'X' : 'x';
        case 0x15: return shift_pressed ? 'Y' : 'y';
        case 0x2C: return shift_pressed ? 'Z' : 'z';


        case 0x39: return ' ';
        case 0x34: return shift_pressed ? '>' : '.';
        case 0x0C: return shift_pressed ? '_' : '-';


        default:
            return 0;
    }
}

void keyboard_handler(void){
    unsigned char scancode=inb(0x60);
    if (scancode==0x1C){
        new_line();
        extract_parms(line);
        clear(&line);
        outb(0x20, 0x20);
    }
    char c=scancode_to_char(scancode);

    if (c!=0){
        
        type(c);
    }
    outb(0x20, 0x20);
}
//input

void load_idt(void){
    struct idt_ptr idtp;

    idtp.limit = sizeof(idt) - 1;
    idtp.base = (unsigned int)&idt;

    __asm__ volatile (
        "lidtl (%0)"
        :
        : "r" (&idtp)
    );
}
unsigned short read_cs(void){
    unsigned short cs;

    __asm__ volatile (
        "mov %%cs, %0"
        : "=r"(cs)
    );

    return cs;
}
/*void read_cmd_queue(){
    for (int i; command_queue.data[i]!='\0'; i++){
        process_command(command_queue.data[i]);
    }
}*/
Array root_subfolders = {0};

Folder root = {0};

void init_filesystem(void)
{
    root.name[0] = '.';
    root.name[1] = '\0';

    root.id = 0;

    root.subfolders = &root_subfolders;
    root.sub_chid = 0;

    root.parent = 0;

    current_folder = &root;

    for (int i = 0; i < CAPACITY; i++) {
        folder_subfolders[i].size = 0;
    }
}
void kernel_main(void){
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
    init_filesystem();


    while (1) {
        __asm__ volatile ("hlt");
    }
}
