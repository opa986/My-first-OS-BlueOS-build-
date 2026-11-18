extern char* v;
extern void print(const char* str, char cor);

void delay_boot(int ciclos) {
    for (volatile int i = 0; i < ciclos * 100000; i++) {
        asm volatile ("nop");
    }
}

void boot_animation() {
    for (int i = 0; i < 4000; i += 2) {
        v[i] = ' ';
        v[i + 1] = 0x1F;
    }

    const char* frames[4] = {
        "  [ *       ] Carregando Kernel x86...      ",
        "  [ ***     ] Inicializando Teclado PS/2... ",
        "  [ ******  ] Montando Jokeware & VGA...    ",
        "  [ ********] Iniciando blueOS 4.0...       "
    };

    for (int f = 0; f < 4; f++) {
        extern int pos;
        pos = 160 * 8 + 32;
        print("  ____  _            ___  ____  ", 0x1E);
        pos = 160 * 9 + 32;
        print(" | __ )| |_   _  ___/ _ \\/ ___| ", 0x1E);
        pos = 160 * 10 + 32;
        print(" |  _ \\| | | | |/ _ \\ | | \\___ \\ ", 0x1B);
        pos = 160 * 11 + 32;
        print(" | |_) | | |_| |  __/ |_| |___) |", 0x1B);
        pos = 160 * 12 + 32;
        print(" |____/|_|\\__,_|\\___|\\___/|____/ ", 0x1F);

        pos = 160 * 15 + 34;
        print(frames[f], 0x1A);
        delay_boot(80);
    }
}
