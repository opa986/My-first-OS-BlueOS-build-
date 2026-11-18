static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

char* v = (char*) 0xb8000;
int pos = 0;
char cor_fundo = 0x10;
char cor_texto = 0x1F;
char cor_destaque = 0x1E;
char cor_prompt = 0x1A;
unsigned int seed = 12345;

char teclado_buffer[128];
int buf_pos = 0;

void delay_vga(int ciclos) {
    for (volatile int i = 0; i < ciclos * 80000; i++) {
        asm volatile ("nop");
    }
}

void reiniciar_sistema() {
    // Dispara um reset real na controladora de teclado 8042 (reinicia o PC de verdade)
    outb(0x64, 0xFE);
    while(1) {
        asm volatile ("cli; hlt");
    }
}

void esperar_enter_ou_reset() {
    while (inb(0x64) & 0x01) { inb(0x60); }
    while (1) {
        if (inb(0x64) & 0x01) {
            unsigned char sc = inb(0x60);
            if (!(sc & 0x80)) {
                // Scancode 28 é o ENTER no teclado PS/2
                if (sc == 28 || sc == 57) { // ENTER ou ESPAÇO reinicia o PC de verdade
                    reiniciar_sistema();
                }
            }
        }
    }
}

void scroll() {
    if (pos >= 4000) {
        for (int i = 160; i < 4000; i++) {
            v[i - 160] = v[i];
        }
        for (int i = 3840; i < 4000; i += 2) {
            v[i] = ' ';
            v[i + 1] = cor_texto;
        }
        pos = 3840;
    }
}

void print(const char* str, char cor) {
    for (int i = 0; str[i] != 0; i++) {
        if (str[i] == '\n') {
            pos = ((pos / 160) + 1) * 160;
        } else {
            v[pos] = str[i];
            v[pos + 1] = cor;
            pos += 2;
        }
        scroll();
    }
}

void print_num(int n, char cor) {
    if (n < 0) {
        print("-", cor);
        n = -n;
    }
    if (n == 0) {
        print("0", cor);
        return;
    }
    char buf[16];
    int i = 0;
    while (n > 0) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    while (i > 0) {
        char c[2] = {buf[--i], 0};
        print(c, cor);
    }
}

void limpar_tela() {
    for (int i = 0; i < 4000; i += 2) {
        v[i] = ' ';
        v[i + 1] = cor_texto;
    }
    pos = 0;
    print("================================================================================\n", cor_destaque);
    print("       blueOS 4.2 - DOS GDI MALWARE & HARD CRASH SUITE (Real Reset)            \n", cor_texto);
    print("================================================================================\n", cor_destaque);
    print("Digite 'jokeware-maker' para abrir o painel de crash real e payloads de 5s.\n\n", cor_destaque);
    print("blueOS-4.2> ", cor_prompt);
}

int iguais(const char* a, const char* b) {
    int i = 0;
    while (a[i] != 0 && b[i] != 0) {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return a[i] == b[i];
}

int comeca_com(const char* str, const char* prefixo) {
    int i = 0;
    while (prefixo[i] != 0) {
        if (str[i] != prefixo[i]) return 0;
        i++;
    }
    return 1;
}

int ler_num(const char* s, int* idx) {
    int res = 0, sinal = 1;
    while (s[*idx] == ' ') (*idx)++;
    if (s[*idx] == '-') { sinal = -1; (*idx)++; }
    while (s[*idx] >= '0' && s[*idx] <= '9') {
        res = res * 10 + (s[*idx] - '0');
        (*idx)++;
    }
    return res * sinal;
}

unsigned char ler_cmos(unsigned char reg) {
    outb(0x70, reg);
    unsigned char val = inb(0x71);
    return ((val / 16) * 10) + (val & 0x0F);
}

unsigned int rand() {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

void mudar_tema(char f, char t, char d, char p) {
    cor_fundo = f; cor_texto = t; cor_destaque = d; cor_prompt = p;
    limpar_tela();
}

void tela_morte_real(char cor_tela, const char* titulo, const char* codigo_erro, const char* desc) {
    // Trava interrupções da CPU (Kernel Panic real)
    asm volatile ("cli");

    for (int i = 0; i < 4000; i += 2) {
        v[i] = ' ';
        v[i + 1] = cor_tela;
    }
    pos = 160 * 3 + 8;
    print(":(   ", cor_tela);
    print(titulo, cor_tela);
    pos = 160 * 6 + 8;
    print("O seu computador sofreu um Kernel Panic real e travou o sistema.", cor_tela);
    pos = 160 * 7 + 8;
    print("Todas as operacoes foram suspensas para proteger o hardware.", cor_tela);
    pos = 160 * 10 + 8;
    print("Erro fatal travado no nucleo (Bare Metal x86)", cor_tela);
    pos = 160 * 13 + 8;
    print("Codigo de parada (Stop Code): ", cor_tela);
    print(codigo_erro, cor_tela);
    pos = 160 * 14 + 8;
    print("Detalhes: ", cor_tela);
    print(desc, cor_tela);
    pos = 160 * 18 + 8;
    print("[HARD CRASH] Pressione ENTER para reiniciar o computador de verdade...", cor_tela);

    // Entra em loop infinito de travamento aguardando o ENTER forçar o reset físico
    esperar_enter_ou_reset();
}

void executar_gdi_malware(int modo) {
    for (int frame = 0; frame < 45; frame++) {
        for (int i = 0; i < 4000; i += 2) {
            if (modo == 1) {
                v[i] = (char)((rand() % 94) + 33);
                v[i + 1] = (char)(rand() % 16);
            } else if (modo == 2) {
                int linha = (i / 160);
                int col = (i % 160) / 2;
                char cor = ((linha + col + frame) % 15) + 1;
                v[i] = (frame % 2 == 0) ? '#' : '*';
                v[i + 1] = (cor << 4) | 0x0F;
            } else if (modo == 3) {
                if (rand() % 3 == 0) {
                    v[i] = (rand() % 2) ? '1' : '0';
                    v[i + 1] = 0x0A;
                } else {
                    v[i] = ' ';
                    v[i + 1] = 0x00;
                }
            } else if (modo == 4) {
                int idx = (i + (frame * 160)) % 4000;
                v[idx] = (char)((rand() % 26) + 'A');
                v[idx + 1] = 0x4F;
            } else if (modo == 5) {
                char flash_attr = (frame % 2 == 0) ? 0x7F : 0x07;
                v[i] = (char)((rand() % 75) + 48);
                v[i + 1] = flash_attr;
            } else if (modo == 6) {
                int col = (i % 160);
                if (col % 4 == 0) { v[i] = '/'; v[i + 1] = 0x0B; }
                else { v[i] = '-'; v[i + 1] = 0x0C; }
            } else if (modo == 7) {
                v[i] = (char)(219);
                v[i + 1] = ((rand() % 2) ? 0x4E : 0x6E);
            } else {
                v[i] = (char)((rand() % 220) + 32);
                v[i + 1] = (char)((rand() % 14) + 1);
            }
        }
        delay_vga(22);
    }
    limpar_tela();
    print("[DOS-GDI-MALWARE] Payload GDI executado por 5s com sucesso!\n", cor_prompt);
}

void executar(const char* cmd);

char scancode_para_ascii(unsigned char scancode) {
    char scancode_ascii[] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    if (scancode < sizeof(scancode_ascii)) {
        return scancode_ascii[scancode];
    }
    return 0;
}

void kernel_main() {
    limpar_tela();
    buf_pos = 0;

    while (1) {
        if (inb(0x64) & 0x01) {
            unsigned char scancode = inb(0x60);
            if (!(scancode & 0x80)) {
                char c = scancode_para_ascii(scancode);
                if (c == '\n') {
                    teclado_buffer[buf_pos] = 0;
                    print("\n", cor_texto);
                    if (buf_pos > 0) {
                        executar(teclado_buffer);
                    }
                    buf_pos = 0;
                    print("blueOS-4.2> ", cor_prompt);
                } else if (c == '\b') {
                    if (buf_pos > 0) {
                        buf_pos--;
                        pos -= 2;
                        v[pos] = ' ';
                        v[pos + 1] = cor_texto;
                    }
                } else if (c >= 32 && c <= 126) {
                    if (buf_pos < 127) {
                        teclado_buffer[buf_pos++] = c;
                        char str[2] = {c, 0};
                        print(str, cor_texto);
                    }
                }
            }
        }
    }
}

void executar(const char* cmd) {
    seed += pos;

    if (iguais(cmd, "help1") || iguais(cmd, "ajuda1")) {
        print("[Ajuda 1/6: Sistema, Info & Cores]\n", cor_destaque);
        print("help1..help6, sobre, versao, autor, empresa, criador, arquitetura, memoria,\n", cor_texto);
        print("cpu, bios, status, uptime, hora, data, calendario, limpar, reiniciar, sair,\n", cor_texto);
        print("tema-azul, tema-matrix, tema-hacker, tema-ouro, tema-roxo, tema-claro, tema-vermelho\n", cor_texto);
    } else if (iguais(cmd, "help2") || iguais(cmd, "ajuda2")) {
        print("[Ajuda 2/6: Matematica Avancada e Operacoes Multiplas]\n", cor_destaque);
        print("soma <a> <b>, soma4, soma6, sub <a> <b>, sub4, mult <a> <b>, mult4, media4,\n", cor_texto);
        print("media6, div <a> <b>, calc <expr>, quad <n>, cubo <n>, tabuada <n>, fatorial <n>\n", cor_texto);
    } else if (iguais(cmd, "help3") || iguais(cmd, "ajuda3")) {
        print("[Ajuda 3/6: Jogos e Sandbox de Fisica / Ragdoll]\n", cor_destaque);
        print("dado, moeda, sorte, adivinhar, jokenpo, ragdoll-box, ragdoll-torture,\n", cor_texto);
        print("ragdoll-explosao, sandbox-fisica, colisao-teste, gravidade-sim\n", cor_texto);
    } else if (iguais(cmd, "help4") || iguais(cmd, "ajuda4")) {
        print("[Ajuda 4/6: Artes ASCII, Entretenimento e Curiosidades]\n", cor_destaque);
        print("banner, foguete, coracao, robo, cafe, caveira, piada1, piada2, curio1,\n", cor_texto);
        print("motivar, historia, missao, matrix, hack, beep, diagnostico\n", cor_texto);
    } else if (iguais(cmd, "help5") || iguais(cmd, "ajuda5")) {
        print("[Ajuda 5/6: Utilitarios e Texto]\n", cor_destaque);
        print("echo <txt>, contar <txt>, ascii, status, memoria, arquitetura\n", cor_texto);
    } else if (iguais(cmd, "help6") || iguais(cmd, "ajuda6")) {
        print("[Ajuda 6/6: HARD CRASH SUITE & GDI MALWARE (Real Crash/Reset)]\n", 0x1C);
        print("jokeware-maker  -> Abre o painel avancado (suporta 1 a 8 ou numeros livres ex: 1637)\n", cor_prompt);
        print("bsod, blsod, rsod, gsod -> Telas de Erro Critico reais com travamento e reset\n", cor_texto);
    } else if (iguais(cmd, "jokeware-maker")) {
        print("====================================================\n", cor_destaque);
        print("       blueOS HARD CRASH & GDI SUITE v4.2           \n", cor_prompt);
        print("====================================================\n", cor_destaque);
        print(" [1] Glitch Memez       | [5] Epilepsy Strobe\n", cor_texto);
        print(" [2] Rainbow Tunel      | [6] Vertical Scanlines\n", cor_texto);
        print(" [3] Binary Matrix      | [7] Fire / Ascensao\n", cor_texto);
        print(" [4] Screen Melt/Invert | [8] BSOD Real (Trava + Reset)\n", cor_texto);
        print("Digite qualquer numero (ex: 1 a 8, ou 1637 para modo caotico) + ENTER: ", cor_destaque);

        char sub_buf[32];
        int sub_len = 0;
        for(int k=0; k<32; k++) sub_buf[k] = 0;

        while (1) {
            if (inb(0x64) & 0x01) {
                unsigned char sc = inb(0x60);
                if (!(sc & 0x80)) {
                    char tecla = scancode_para_ascii(sc);
                    if (tecla == '\n') {
                        print("\n", cor_texto);
                        break;
                    } else if (tecla == '\b') {
                        if (sub_len > 0) {
                            sub_len--;
                            pos -= 2;
                            v[pos] = ' ';
                            v[pos + 1] = cor_texto;
                        }
                    } else if (tecla >= '0' && tecla <= '9') {
                        if (sub_len < 31) {
                            sub_buf[sub_len++] = tecla;
                            char s_str[2] = {tecla, 0};
                            print(s_str, cor_texto);
                        }
                    }
                }
            }
        }

        int escolha = 0;
        for (int i = 0; sub_buf[i] != 0; i++) {
            escolha = escolha * 10 + (sub_buf[i] - '0');
        }

        if (escolha == 1) {
            executar_gdi_malware(1);
        } else if (escolha == 2) {
            executar_gdi_malware(2);
        } else if (escolha == 3) {
            executar_gdi_malware(3);
        } else if (escolha == 4) {
            executar_gdi_malware(4);
        } else if (escolha == 5) {
            executar_gdi_malware(5);
        } else if (escolha == 6) {
            executar_gdi_malware(6);
        } else if (escolha == 7) {
            executar_gdi_malware(7);
        } else if (escolha == 8) {
            tela_morte_real(0x1F, "BLUE SCREEN OF DEATH (BSOD)", "CRITICAL_PROCESS_DIED", "Kernel Panic real forçado pelo Jokeware Maker");
        } else {
            print("Numero livre detectado! Ativando modo Wildcard GDI Maligno (1637 Style)...\n", 0x1C);
            delay_vga(30);
            executar_gdi_malware(9);
        }
        return;
    } else if (iguais(cmd, "bsod")) {
        tela_morte_real(0x1F, "BLUE SCREEN OF DEATH (BSOD)", "CRITICAL_PROCESS_DIED (0x000000EF)", "Falha critica real no nucleo do sistema blueOS");
        return;
    } else if (iguais(cmd, "blsod")) {
        tela_morte_real(0x0F, "BLACK SCREEN OF DEATH (BLSOD)", "SYSTEM_THREAD_EXCEPTION_NOT_HANDLED", "Erro fatal de excecao em thread de sistema");
        return;
    } else if (iguais(cmd, "rsod")) {
        tela_morte_real(0x4F, "RED SCREEN OF DEATH (RSOD)", "BOOT_MGR_CRITICAL_HARDWARE_FAULT", "Falha critica de hardware em nivel de firmware");
        return;
    } else if (iguais(cmd, "gsod")) {
        tela_morte_real(0x2F, "GREEN SCREEN OF DEATH (GSOD)", "INSIDER_BUILD_KERNEL_SECURITY_CHECK", "Falha de validacao de seguranca em build de nucleo");
        return;
    } else if (iguais(cmd, "sobre")) {
        print("blueOS 4.2 Release - O DOS mais leve do universo com Crash Real!\n", cor_prompt);
    } else if (iguais(cmd, "versao")) {
        print("blueOS Kernel v4.2.0-HARD-CRASH (x86 32-bit Protected Mode Bare Metal)\n", cor_texto);
    } else if (iguais(cmd, "autor") || iguais(cmd, "criador")) {
        print("Criador & Arquiteto: Luzuky / R6K - Programador de 10 anos de Minas Gerais!\n", cor_prompt);
    } else if (iguais(cmd, "empresa")) {
        print("Luzuky Tech / blueOS Corp - Tecnologia independente sem base Linux/Windows.\n", cor_destaque);
    } else if (iguais(cmd, "arquitetura")) {
        print("Arquitetura x86 Assembly (NASM) + C Freestanding (GCC -m32).\n", cor_texto);
    } else if (iguais(cmd, "memoria")) {
        print("Mapeamento de Video VGA em 0xB8000 | Pilha de Kernel em 16KB.\n", cor_texto);
    } else if (iguais(cmd, "cpu")) {
        print("Processador Virtualizado via QEMU (Modo Protegido 32-bits).\n", cor_texto);
    } else if (iguais(cmd, "status")) {
        print("[OK] Kernel v4.2 | [OK] Hard Crash Engine | [OK] Real Reset Controller\n", cor_prompt);
    } else if (iguais(cmd, "uptime")) {
        print("Tempo de execucao do sistema: estavel e sem interrupcoes.\n", cor_texto);
    } else if (iguais(cmd, "hora")) {
        print("Horario atual (BIOS RTC): ", cor_destaque);
        print_num(ler_cmos(0x04), cor_texto); print(":", cor_texto);
        print_num(ler_cmos(0x02), cor_texto); print(":", cor_texto);
        print_num(ler_cmos(0x00), cor_texto); print("\n", cor_texto);
    } else if (iguais(cmd, "data")) {
        print("Data atual (BIOS RTC): ", cor_destaque);
        print_num(ler_cmos(0x07), cor_texto); print("/", cor_texto);
        print_num(ler_cmos(0x08), cor_texto); print("/2026\n", cor_texto);
    } else if (iguais(cmd, "limpar") || iguais(cmd, "cls")) {
        limpar_tela(); return;
    } else if (iguais(cmd, "tema-azul")) {
        mudar_tema(0x10, 0x1F, 0x1E, 0x1A); return;
    } else if (iguais(cmd, "tema-matrix")) {
        mudar_tema(0x00, 0x0A, 0x02, 0x0F); return;
    } else if (iguais(cmd, "tema-hacker")) {
        mudar_tema(0x00, 0x0B, 0x0E, 0x0C); return;
    } else if (iguais(cmd, "tema-ouro")) {
        mudar_tema(0x00, 0x0E, 0x0F, 0x0C); return;
    } else if (iguais(cmd, "tema-roxo")) {
        mudar_tema(0x50, 0x5F, 0x5E, 0x5B); return;
    } else if (iguais(cmd, "tema-claro")) {
        mudar_tema(0x70, 0x70, 0x71, 0x74); return;
    } else if (iguais(cmd, "tema-vermelho")) {
        mudar_tema(0x40, 0x4F, 0x4E, 0x4B); return;
    } else if (comeca_com(cmd, "soma ")) {
        int i = 5; int a = ler_num(cmd, &i); int b = ler_num(cmd, &i);
        print("Soma: ", cor_destaque); print_num(a + b, cor_texto); print("\n", cor_texto);
    } else if (comeca_com(cmd, "sub ")) {
        int i = 4; int a = ler_num(cmd, &i); int b = ler_num(cmd, &i);
        print("Subtracao: ", cor_destaque); print_num(a - b, cor_texto); print("\n", cor_texto);
    } else if (comeca_com(cmd, "mult ")) {
        int i = 5; int a = ler_num(cmd, &i); int b = ler_num(cmd, &i);
        print("Multiplicacao: ", cor_destaque); print_num(a * b, cor_texto); print("\n", cor_texto);
    } else if (comeca_com(cmd, "div ")) {
        int i = 4; int a = ler_num(cmd, &i); int b = ler_num(cmd, &i);
        if (b == 0) print("Erro: Divisao por zero!\n", 0x1C);
        else { print("Divisao: ", cor_destaque); print_num(a / b, cor_texto); print("\n", cor_texto); }
    } else if (iguais(cmd, "dado")) {
        int r = (rand() % 6) + 1;
        print("[JOGO] Dado rolado (1-6): ", cor_destaque); print_num(r, cor_prompt); print("\n", cor_texto);
    } else if (iguais(cmd, "moeda")) {
        int r = rand() % 2;
        if (r == 0) print("[JOGO] Moeda: CARA!\n", cor_prompt);
        else print("[JOGO] Moeda: COROA!\n", cor_destaque);
    } else if (iguais(cmd, "ragdoll-box")) {
        print("[SANDBOX] Ragdoll Physics & Gore Engine Ativa!\n", 0x1C);
        print("      O      \n     /|\\     \n      / \\    \n=== Fraturas registradas na memoria VGA ===\n", cor_destaque);
    } else if (iguais(cmd, "banner")) {
        print("  ____  _            ___  ____  \n | __ )| |_   _  ___/ _ \\/ ___| \n |  _ \\| | | | |/ _ \\ | | \\___ \\ \n | |_) | | |_| |  __/ |_| |___) |\n |____/|_|\\__,_|\\___|\\___/|____/ \n", cor_destaque);
    } else if (iguais(cmd, "foguete")) {
        print("        ^        \n       / \\       \n      /___\\      \n     |     |     \n     |blueOS|    \n", cor_destaque);
    } else if (iguais(cmd, "reiniciar")) {
        print("Reiniciando sistema...\n", cor_destaque);
        reiniciar_sistema();
    } else if (iguais(cmd, "sair")) {
        print("Sistema haltado com seguranca.\n", cor_destaque);
        asm volatile ("cli; hlt");
    } else {
        print("Comando desconhecido! Digite 'help1' ate 'help6' ou 'jokeware-maker'.\n", 0x1C);
    }
}
