# BlueOS

> O sistema operacional mais leve do universo. Um OS bare-metal em x86 32-bit construído do zero em Assembly (NASM) e C puro, sem dependências de Linux ou Windows.

---

##  Sobre o Projeto

O **BlueOS** é um projeto de sistema operacional focado em baixo nível, eficiência extrema e controle absoluto do hardware. Ele roda direto no metal (*bare-metal*), comunicando-se diretamente com a memória de vídeo VGA (`0xB8000`) e com as controladoras do sistema.

Com um kernel extremamente otimizado, o sistema inteiro e a imagem `.iso` bootável pesam cerca de **1 MB**, contendo mais de 160 comandos de terminal, suíte GDI de efeitos visuais e gerenciamento de crash real com reset físico.

---

##  Principais Funcionalidades

* **Arquitetura x86 Bare-Metal:** Inicialização via Multiboot spec (`boot.s`) com kernel freestanding em C.
* **Interface VGA Direta:** Renderização ultrarrápida direto na memória de vídeo sem drivers pesados.
* **Driver de Teclado PS/2 Nativo:** Suporte completo a buffer de entrada e leitura de scancodes.
* **Mais de 160 Comandos Nativos:** Utilitários de sistema, matemática avançada, lógica, jogos, arte ASCII e curiosidades.
* **DOS-GDI-Malware Suite (`jokeware-maker`):** Suporte a entrada numérica estilo DOS (suportando valores livres como `1637`) e payloads visuais simulados (Glitch, Rainbow Tunnel, Binary Matrix, Screen Melt, Strobe, Scanlines, Fire) rodando por 5 segundos.
* **Hard Crash & Real Reset:** Telas de erro críticas (`BSOD`, `BLSOD`, `RSOD`, `GSOD`) com travamento real de interrupções (`cli`) e reinicialização física via controladora de teclado 8042 (`0xFE`).
* **RTC CMOS Integration:** Leitura de relógio e calendário direto da BIOS em tempo real.

---

##  Tecnologias Utilizadas

* **Assembly x86 (NASM):** Bootloader e configuração inicial em modo protegido de 32-bits.
* **C (GCC):** Lógica do kernel, shell, gerenciamento de vídeo e interpretador de comandos (`-ffreestanding -m32`).
* **GNU Make / Linker Script (`linker.ld`):** Layout de memória customizado a partir de `1M`.
* **GRUB & Xorriso (`grub-mkrescue`):** Empacotamento da imagem `.iso` bootável.
* **QEMU:** Ambiente de virtualização para testes rápidos.

---

##  Como Compilar e Rodar

Certifique-se de ter as ferramentas necessárias instaladas no seu ambiente Linux (GCC, NASM, QEMU, Xorriso e GRUB):

```bash
# Instalar dependências (Ubuntu/Debian/Mint)
sudo apt update && sudo apt install build-essential nasm qemu-system-x86 xorriso grub-pc-bin

# Clonar ou criar a pasta de desenvolvimento
mkdir -p ~/blueOS && cd ~/blueOS

# Compilar os arquivos do kernel, gerar a ISO e iniciar no QEMU
# (Utilize o script de automação integrado do blueOS)
