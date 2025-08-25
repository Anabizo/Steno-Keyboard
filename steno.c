#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/uinput.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bizo");
MODULE_DESCRIPTION("Stenography Keyboard - Fase 1");

#define MAX_BUFFER_SIZE 64

static struct input_dev *virtual_dev = NULL;

struct kb_handle {
    struct input_handle ih;
    bool processing;
    char buffer[MAX_BUFFER_SIZE + 1];
    int buffer_pos;
};

static void clear_buffer(struct kb_handle *kh);
static void inject_string(const char* str);

static void clear_buffer(struct kb_handle *kh)
{
    kh->buffer_pos = 0;
    kh->buffer[0] = '\0';
    pr_info("stenography: buffer limpo\n");
}

static bool add_char_to_buffer(struct kb_handle *kh, char c)
{
    if (kh->buffer_pos >= MAX_BUFFER_SIZE) {
        pr_warn("stenography: buffer cheio, limpando automaticamente\n");
        clear_buffer(kh);
    }
   
    kh->buffer[kh->buffer_pos] = c;
    kh->buffer_pos++;
    kh->buffer[kh->buffer_pos] = '\0';
   
    pr_info("stenography: buffer agora tem '%s' (%d chars)\n", kh->buffer, kh->buffer_pos);
   
    return true;
}

static const char* get_buffer_content(struct kb_handle *kh)
{
    return kh->buffer;
}

static bool is_buffer_empty(struct kb_handle *kh)
{
    return (kh->buffer_pos == 0);
}

static bool remove_last_char_from_buffer(struct kb_handle *kh)
{
    if (is_buffer_empty(kh)) {
        pr_info("stenography: backspace ignorado - buffer já está vazio\n");
        return false;
    }
   
    kh->buffer_pos--;
    kh->buffer[kh->buffer_pos] = '\0';
   
    pr_info("stenography: backspace aplicado - buffer agora: '%s' (%d chars)\n",
            kh->buffer, kh->buffer_pos);
   
    return true;
}

static void inject_string(const char* str)
{
    int i;
    int len = strlen(str);
    
    if (!virtual_dev) {
        pr_err("stenography: dispositivo virtual não disponível\n");
        return;
    }
    
    pr_info("stenography: injetando string: '%s'\n", str);
    
    static const struct {
        char c;
        int keycode;
    } char_to_keycode[] = {
        {'q', KEY_Q}, {'w', KEY_W}, {'e', KEY_E}, {'r', KEY_R},
        {'t', KEY_T}, {'y', KEY_Y}, {'u', KEY_U}, {'i', KEY_I},
        {'o', KEY_O}, {'p', KEY_P},
        {'a', KEY_A}, {'s', KEY_S}, {'d', KEY_D}, {'f', KEY_F},
        {'g', KEY_G}, {'h', KEY_H}, {'j', KEY_J}, {'k', KEY_K},
        {'l', KEY_L},
        {'z', KEY_Z}, {'x', KEY_X}, {'c', KEY_C}, {'v', KEY_V},
        {'b', KEY_B}, {'n', KEY_N}, {'m', KEY_M},
        {' ', KEY_SPACE}, {'\n', KEY_ENTER}
    };
    
    for (i = 0; i < len; i++) {
        char c = str[i];
        int keycode = 0;
        int j;
        
        for (j = 0; j < ARRAY_SIZE(char_to_keycode); j++) {
            if (char_to_keycode[j].c == c) {
                keycode = char_to_keycode[j].keycode;
                break;
            }
        }
        
        if (keycode == 0) {
            pr_warn("stenography: caractere '%c' não encontrado no mapeamento\n", c);
            continue;
        }
        
        pr_debug("stenography: injetando '%c' -> keycode %d\n", c, keycode);
        
        input_report_key(virtual_dev, keycode, 1);
        input_sync(virtual_dev);
        
        input_report_key(virtual_dev, keycode, 0);
        input_sync(virtual_dev);
    }
}

static bool swap_filter(struct input_handle *ih,
                        unsigned int type, unsigned int code, int value)
{
    struct kb_handle *kh = container_of(ih, struct kb_handle, ih);
    char captured_char = 0;

    pr_debug("stenography: EVENT type=%u code=%u value=%d\n", type, code, value);

    if (type != EV_KEY || value != 1)
        return false;

    if (kh->processing)
        return false;

    static const char keycode_to_char[] = {
        [KEY_Q] = 'q', [KEY_W] = 'w', [KEY_E] = 'e', [KEY_R] = 'r',
        [KEY_T] = 't', [KEY_Y] = 'y', [KEY_U] = 'u', [KEY_I] = 'i',
        [KEY_O] = 'o', [KEY_P] = 'p',
        [KEY_A] = 'a', [KEY_S] = 's', [KEY_D] = 'd', [KEY_F] = 'f',
        [KEY_G] = 'g', [KEY_H] = 'h', [KEY_J] = 'j', [KEY_K] = 'k',
        [KEY_L] = 'l',
        [KEY_Z] = 'z', [KEY_X] = 'x', [KEY_C] = 'c', [KEY_V] = 'v',
        [KEY_B] = 'b', [KEY_N] = 'n', [KEY_M] = 'm',
        [KEY_LEFTSHIFT] = 0, [KEY_RIGHTSHIFT] = 0,
        [KEY_SPACE] = ' ',
        [KEY_BACKSPACE] = '\b',
        [KEY_ENTER] = '\n'
    };

    if (code < ARRAY_SIZE(keycode_to_char)) {
        captured_char = keycode_to_char[code];
    }

    if (captured_char) {
        pr_info("stenography: processando tecla 0x%x -> '%c'\n", code, captured_char);
        
        kh->processing = true;
       
        switch (captured_char) {
            case ' ':  
                if (!is_buffer_empty(kh)) {
                    pr_info("stenography: espaço detectado - injetando palavra: '%s'\n",
                            get_buffer_content(kh));
                    inject_string(get_buffer_content(kh));
                    inject_string(" ");
                } else {
                    pr_info("stenography: espaço com buffer vazio - injetando espaço normal\n");
                    inject_string(" ");
                }
                clear_buffer(kh);
                kh->processing = false;
                return true;
               
            case '\b':
                if (remove_last_char_from_buffer(kh)) {
                    pr_info("stenography: backspace processado - bloqueando tecla original\n");
                } else {
                    pr_info("stenography: backspace em buffer vazio - passando backspace normal\n");
                    kh->processing = false;
                    return false;
                }
                kh->processing = false;
                return true;
               
            case '\n':
                pr_info("stenography: enter detectado - limpando buffer e passando enter\n");
                clear_buffer(kh);
                kh->processing = false;
                return false;
               
            default:
                add_char_to_buffer(kh, captured_char);
                pr_info("stenography: caractere '%c' adicionado ao buffer - BLOQUEANDO tecla\n", captured_char);
                kh->processing = false;
                return true;
        }
    }

    return false;
}

static int swap_connect(struct input_handler *handler, struct input_dev *dev,
                        const struct input_device_id *id)
{
    struct kb_handle *kh;
    int i;
    unsigned long keybit[BITS_TO_LONGS(KEY_CNT)] = { 0 };
   
    pr_info("stenography: analisando dispositivo '%s'\n", dev->name);
   
    if (dev == virtual_dev) {
        pr_info("stenography: ignorando nosso dispositivo virtual\n");
        return -ENODEV;
    }

    if (!test_bit(EV_KEY, dev->evbit)) {
        pr_info("stenography: dispositivo não suporta EV_KEY\n");
        return -ENODEV;
    }

    memcpy(keybit, dev->keybit, sizeof(keybit));
   
    if (!test_bit(KEY_A, keybit) || !test_bit(KEY_Q, keybit)) {
        pr_info("stenography: faltam teclas essenciais (A: %d, Q: %d)\n",
                test_bit(KEY_A, keybit), test_bit(KEY_Q, keybit));
        return -ENODEV;
    }

    kh = kzalloc(sizeof(*kh), GFP_KERNEL);
    if (!kh) {
        pr_err("stenography: falha ao alocar memória\n");
        return -ENOMEM;
    }

    kh->ih.dev = dev;
    kh->ih.handler = handler;
    kh->ih.name = "stenography_handle";
    kh->processing = false;
    clear_buffer(kh);

    if (input_register_handle(&kh->ih)) {
        pr_err("stenography: falha ao registrar handle\n");
        kfree(kh);
        return -ENODEV;
    }

    if (input_open_device(&kh->ih)) {
        pr_err("stenography: falha ao abrir dispositivo\n");
        input_unregister_handle(&kh->ih);
        kfree(kh);
        return -ENODEV;
    }

    pr_info("stenography: conectado com sucesso a '%s'\n", dev_name(&dev->dev));
   
    pr_info("stenography: teclas suportadas (A-Z): ");
    for (i = KEY_A; i <= KEY_Z; i++) {
        if (test_bit(i, keybit)) {
            pr_cont("%c ", 'a' + (i - KEY_A));
        }
    }
    pr_cont("\n");

    return 0;
}

static void swap_disconnect(struct input_handle *ih)
{
    struct kb_handle *kh = container_of(ih, struct kb_handle, ih);
   
    pr_info("stenography: desconectando de %s\n", dev_name(&ih->dev->dev));
   
    input_close_device(&kh->ih);
    input_unregister_handle(&kh->ih);
    kfree(kh);
}

static const struct input_device_id swap_ids[] = {
    {
        .flags = INPUT_DEVICE_ID_MATCH_EVBIT,
        .evbit = { BIT_MASK(EV_KEY) },
    },
    { },
};

MODULE_DEVICE_TABLE(input, swap_ids);

static struct input_handler swap_handler = {
    .filter     = swap_filter,
    .connect    = swap_connect,
    .disconnect = swap_disconnect,
    .name       = "stenography",
    .id_table   = swap_ids,
};

static int create_virtual_device(void)
{
    int ret;
   
    virtual_dev = input_allocate_device();
    if (!virtual_dev) {
        pr_err("stenography: falha ao alocar dispositivo virtual\n");
        return -ENOMEM;
    }
   
    virtual_dev->name = "Stenography Virtual Keyboard";
    virtual_dev->id.bustype = BUS_VIRTUAL;
    virtual_dev->id.vendor  = 0x0001;
    virtual_dev->id.product = 0x0001;
    virtual_dev->id.version = 0x0001;
   
    set_bit(EV_KEY, virtual_dev->evbit);
    set_bit(EV_SYN, virtual_dev->evbit);

    pr_info("stenography: registrando teclas virtuais:\n");
   
    int keys_to_register[] = {
        KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
        KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L,
        KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M,
        KEY_SPACE, KEY_ENTER, KEY_BACKSPACE
    };
    
    for (int i = 0; i < ARRAY_SIZE(keys_to_register); i++) {
        set_bit(keys_to_register[i], virtual_dev->keybit);
        pr_info("stenography: - keycode %d registrado\n", keys_to_register[i]);
    }

    ret = input_register_device(virtual_dev);
    if (ret) {
        pr_err("stenography: falha ao registrar dispositivo virtual (%d)\n", ret);
        input_free_device(virtual_dev);
        virtual_dev = NULL;
        return ret;
    }
   
    pr_info("stenography: dispositivo virtual criado com sucesso\n");
    return 0;
}

static void destroy_virtual_device(void)
{
    if (virtual_dev) {
        pr_info("stenography: destruindo dispositivo virtual\n");
        input_unregister_device(virtual_dev);
        virtual_dev = NULL;
        pr_info("stenography: dispositivo virtual destruído\n");
    }
}

static int __init swap_init(void)
{
    int ret;
   
    pr_info("stenography: ===== INICIALIZANDO MÓDULO STENOGRAPHY - FASE 1 =====\n");
   
    pr_info("stenography: criando dispositivo virtual...\n");
    ret = create_virtual_device();
    if (ret) {
        pr_err("stenography: falha na criação do dispositivo virtual\n");
        return ret;
    }
   
    pr_info("stenography: registrando handler no sistema de input...\n");
    ret = input_register_handler(&swap_handler);
    if (ret) {
        pr_err("stenography: falha ao registrar handler (%d)\n", ret);
        destroy_virtual_device();
        return ret;
    }
   
    pr_info("stenography: ===== MÓDULO CARREGADO COM SUCESSO =====\n");
    pr_info("stenography: FASE 1 - CARACTERES BLOQUEADOS:\n");
    pr_info("stenography: - Letras: bloqueadas, armazenadas no buffer\n");
    pr_info("stenography: - Espaço: injeta buffer + espaço e limpa buffer\n");
    pr_info("stenography: - Backspace: remove do buffer (bloqueado quando há conteúdo)\n");
    pr_info("stenography: - Enter: limpa buffer e passa enter normal\n");
    pr_info("stenography: Use 'dmesg -w' para ver eventos em tempo real\n");
   
    return 0;
}

static void __exit swap_exit(void)
{
    pr_info("stenography: ===== DESCARREGANDO MÓDULO =====\n");
    pr_info("stenography: desregistrando handler...\n");
    input_unregister_handler(&swap_handler);
    pr_info("stenography: destruindo dispositivo virtual...\n");
    destroy_virtual_device();
    pr_info("stenography: ===== MÓDULO DESCARREGADO =====\n");
}

module_init(swap_init);
module_exit(swap_exit);