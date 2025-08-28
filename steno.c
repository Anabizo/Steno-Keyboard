#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/uinput.h>
#include <linux/string.h>
#include <linux/sort.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bizo");
MODULE_DESCRIPTION("Stenography Keyboard");

#define MAX_BUFFER_SIZE 64
#define MAX_WORD_LENGTH 64

struct steno_entry {
    char abbreviation[16];  
    char expansion[MAX_WORD_LENGTH]; 
};

static const struct steno_entry steno_dict[] = {
    {"ces", "secretaria"},
    {"cv", "voce"},
    {"pq", "porque"},
    {"rv", "verao"},
    {"sv", "veroes"},
    {"ap", "praia"},
    {"cs", "casa"},
    {"q", "que"},
    {"n", "nao"},
    {"e", "era"},
    {"a", "ate"},
    {"m", "mim"},
    {"c", "Como"},
    {"mn", "Minha"},
    {"cm", "como"},
    {"vv", "vivesse"},
    {"dv", "verdade"},
    {"ex", "existisse"},
    {"et", "estar"},
    {"er", "realmente"},
    {"im", "importava"},
    {"in", "inverno"},
    {"iv", "vida"},
    {"ju", "junho"},
    {"pr", "Para"},
    {"qs", "quase"},
    {"an", "naquela"},
    {"cn", "contada"},
    {"bm", "Bom dia"}
};

#define DICT_SIZE ARRAY_SIZE(steno_dict)

static struct input_dev *virtual_dev = NULL;

struct kb_handle {
    struct input_handle ih;
    bool processing;
    char buffer[MAX_BUFFER_SIZE + 1];
    int buffer_pos;
    bool shift_pressed;
};

static void clear_buffer(struct kb_handle *kh);
static void inject_string(const char* str);
static const char* search_dictionary(const char* word);

static void clear_buffer(struct kb_handle *kh)
{
    kh->buffer_pos = 0;
    kh->buffer[0] = '\0';
    pr_info("stenography: buffer limpo\n");
}

static int compare_chars(const void *a, const void *b)
{
    return (*(char*)a - *(char*)b);
}

static const char* search_dictionary(const char* word)
{
    char sorted_word[256]; 
    int i, len;
    
    if (!word || strlen(word) == 0) {
        pr_info("stenography: palavra vazia para busca\n");
        return NULL;
    }
    
    len = strlen(word);
    strncpy(sorted_word, word, len);
    sorted_word[len] = '\0';
    
    pr_info("stenography: palavra original: '%s'\n", word);
    sort(sorted_word, len, sizeof(char), compare_chars, NULL);
    pr_info("stenography: palavra ordenada: '%s'\n", sorted_word);
    
    pr_info("stenography: buscando '%s' no dicionario...\n", sorted_word);
    
    for (i = 0; i < DICT_SIZE; i++) {
        if (strcmp(steno_dict[i].abbreviation, sorted_word) == 0) {
            pr_info("stenography: ENCONTRADO! '%s' -> '%s'\n",
                    sorted_word, steno_dict[i].expansion);
            return steno_dict[i].expansion;
        }
    }
    
    pr_info("stenography: '%s' nao encontrado no dicionario\n", sorted_word);
    return NULL;
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
        pr_info("stenography: backspace ignorado - buffer ja esta vazio\n");
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
        pr_err("stenography: dispositivo virtual nao disponivel\n");
        return;
    }
    
    pr_info("stenography: injetando string: '%s'\n", str);
    
    static const struct {
        char c;
        int keycode;
        bool needs_shift;
    } char_to_keycode[] = {
        {'a', KEY_A, false}, {'b', KEY_B, false}, {'c', KEY_C, false},
        {'d', KEY_D, false}, {'e', KEY_E, false}, {'f', KEY_F, false},
        {'g', KEY_G, false}, {'h', KEY_H, false}, {'i', KEY_I, false},
        {'j', KEY_J, false}, {'k', KEY_K, false}, {'l', KEY_L, false},
        {'m', KEY_M, false}, {'n', KEY_N, false}, {'o', KEY_O, false},
        {'p', KEY_P, false}, {'q', KEY_Q, false}, {'r', KEY_R, false},
        {'s', KEY_S, false}, {'t', KEY_T, false}, {'u', KEY_U, false},
        {'v', KEY_V, false}, {'w', KEY_W, false}, {'x', KEY_X, false},
        {'y', KEY_Y, false}, {'z', KEY_Z, false},
        {'A', KEY_A, true}, {'B', KEY_B, true}, {'C', KEY_C, true},
        {'D', KEY_D, true}, {'E', KEY_E, true}, {'F', KEY_F, true},
        {'G', KEY_G, true}, {'H', KEY_H, true}, {'I', KEY_I, true},
        {'J', KEY_J, true}, {'K', KEY_K, true}, {'L', KEY_L, true},
        {'M', KEY_M, true}, {'N', KEY_N, true}, {'O', KEY_O, true},
        {'P', KEY_P, true}, {'Q', KEY_Q, true}, {'R', KEY_R, true},
        {'S', KEY_S, true}, {'T', KEY_T, true}, {'U', KEY_U, true},
        {'V', KEY_V, true}, {'W', KEY_W, true}, {'X', KEY_X, true},
        {'Y', KEY_Y, true}, {'Z', KEY_Z, true},
        {' ', KEY_SPACE, false}, {'\n', KEY_ENTER, false}
    };
    
    for (i = 0; i < len; i++) {
        int keycode = 0;
        bool needs_shift = false;
        
        for (int j = 0; j < ARRAY_SIZE(char_to_keycode); j++) {
            if (char_to_keycode[j].c == str[i]) {
                keycode = char_to_keycode[j].keycode;
                needs_shift = char_to_keycode[j].needs_shift;
                break;
            }
        }
        
        if (keycode == 0) {
            pr_warn("stenography: caractere '%c' (0x%02x) nao mapeado\n", str[i], (unsigned char)str[i]);
            continue;
        }
        
        if (needs_shift) {
            input_report_key(virtual_dev, KEY_LEFTSHIFT, 1);
            input_sync(virtual_dev);
        }
        
        input_report_key(virtual_dev, keycode, 1);
        input_sync(virtual_dev);
        input_report_key(virtual_dev, keycode, 0);
        input_sync(virtual_dev);
        
        if (needs_shift) {
            input_report_key(virtual_dev, KEY_LEFTSHIFT, 0);
            input_sync(virtual_dev);
        }
    }
}

static bool swap_filter(struct input_handle *ih,
                        unsigned int type, unsigned int code, int value)
{
    struct kb_handle *kh = container_of(ih, struct kb_handle, ih);
    char captured_char = 0;

    pr_debug("stenography: EVENT type=%u code=%u value=%d\n", type, code, value);

    if (type != EV_KEY)
        return false;

    if (kh->processing)
        return false;

    if (code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT) {
        kh->shift_pressed = (value == 1);
        pr_info("stenography: shift %s\n", kh->shift_pressed ? "pressionado" : "solto");
        return false;
    }

    if (value != 1)
        return false;

    static const char keycode_to_char_lower[] = {
        [KEY_Q] = 'q', [KEY_W] = 'w', [KEY_E] = 'e', [KEY_R] = 'r',
        [KEY_T] = 't', [KEY_Y] = 'y', [KEY_U] = 'u', [KEY_I] = 'i',
        [KEY_O] = 'o', [KEY_P] = 'p',
        [KEY_A] = 'a', [KEY_S] = 's', [KEY_D] = 'd', [KEY_F] = 'f',
        [KEY_G] = 'g', [KEY_H] = 'h', [KEY_J] = 'j', [KEY_K] = 'k',
        [KEY_L] = 'l',
        [KEY_Z] = 'z', [KEY_X] = 'x', [KEY_C] = 'c', [KEY_V] = 'v',
        [KEY_B] = 'b', [KEY_N] = 'n', [KEY_M] = 'm',
        [KEY_SPACE] = ' ',
        [KEY_BACKSPACE] = '\b',
        [KEY_ENTER] = '\n'
    };

    if (code < ARRAY_SIZE(keycode_to_char_lower) && keycode_to_char_lower[code]) {
        captured_char = keycode_to_char_lower[code];
        
        if (kh->shift_pressed && captured_char >= 'a' && captured_char <= 'z') {
            captured_char = captured_char - 'a' + 'A';
        }
    }

    if (captured_char) {
        pr_info("stenography: processando tecla 0x%x -> '%c' (shift: %s)\n", 
                code, captured_char, kh->shift_pressed ? "sim" : "nao");
        
        kh->processing = true;
       
        switch (captured_char) {
            case ' ':  
                if (!is_buffer_empty(kh)) {
                    const char* buffer_content = get_buffer_content(kh);
                    const char* expanded_word;
                    
                    pr_info("stenography: espaco detectado - processando palavra: '%s'\n", buffer_content);
                    
                    expanded_word = search_dictionary(buffer_content);
                    
                    if (expanded_word) {
                        pr_info("stenography: injetando palavra expandida: '%s'\n", expanded_word);
                        inject_string(expanded_word);
                        inject_string(" ");
                    } else {
                        pr_info("stenography: nao encontrou no dicionario - injetando original: '%s'\n", buffer_content);
                        inject_string(buffer_content);
                        inject_string(" ");
                    }
                } else {
                    pr_info("stenography: espaco com buffer vazio - injetando espaco normal\n");
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
                char buffer_char = (captured_char >= 'A' && captured_char <= 'Z') ? 
                                   (captured_char - 'A' + 'a') : captured_char;
                add_char_to_buffer(kh, buffer_char);
                pr_info("stenography: caractere '%c' adicionado ao buffer como '%c' - BLOQUEANDO tecla\n", 
                        captured_char, buffer_char);
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
        pr_info("stenography: dispositivo nao suporta EV_KEY\n");
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
        pr_err("stenography: falha ao alocar memoria\n");
        return -ENOMEM;
    }

    kh->ih.dev = dev;
    kh->ih.handler = handler;
    kh->ih.name = "stenography_handle";
    kh->processing = false;
    kh->shift_pressed = false;
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
        KEY_SPACE, KEY_ENTER, KEY_BACKSPACE,
        KEY_LEFTSHIFT, KEY_RIGHTSHIFT
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
        pr_info("stenography: dispositivo virtual destruido\n");
    }
}

static int __init swap_init(void)
{
    int ret;
   
    pr_info("stenography: ===== INICIALIZANDO MODULO STENOGRAPHY =====\n");
   
    pr_info("stenography: criando dispositivo virtual...\n");
    ret = create_virtual_device();
    if (ret) {
        pr_err("stenography: falha na criacao do dispositivo virtual\n");
        return ret;
    }
   
    pr_info("stenography: registrando handler no sistema de input...\n");
    ret = input_register_handler(&swap_handler);
    if (ret) {
        pr_err("stenography: falha ao registrar handler (%d)\n", ret);
        destroy_virtual_device();
        return ret;
    }
   
    pr_info("stenography: ===== MODULO CARREGADO COM SUCESSO =====\n");
    pr_info("stenography: - Letras: bloqueadas, armazenadas no buffer (maiusculas suportadas)\n");
    pr_info("stenography: - Espaco: busca palavra no dicionario e injeta resultado\n");
    pr_info("stenography: - Backspace: remove do buffer (bloqueado quando ha conteudo)\n");
    pr_info("stenography: - Enter: limpa buffer e passa enter normal\n");
    pr_info("stenography: \n");
    pr_info("stenography: DICIONARIO DE TESTE (%lu entradas):\n", DICT_SIZE);
    for (int i = 0; i < DICT_SIZE; i++) {
        pr_info("stenography: - '%s' -> '%s'\n", 
                steno_dict[i].abbreviation, steno_dict[i].expansion);
    }
    pr_info("stenography: \n");
    pr_info("stenography: EXEMPLOS DE USO:\n");
    pr_info("stenography: - Digite 'sv' + espaco = 'veroes '\n");
    pr_info("stenography: Use 'make test' para ver eventos em tempo real\n");
   
    return 0;
}

static void __exit swap_exit(void)
{
    pr_info("stenography: ===== DESCARREGANDO MODULO =====\n");
    pr_info("stenography: desregistrando handler...\n");
    input_unregister_handler(&swap_handler);
    pr_info("stenography: destruindo dispositivo virtual...\n");
    destroy_virtual_device();
    pr_info("stenography: ===== MODULO DESCARREGADO =====\n");
}

module_init(swap_init);
module_exit(swap_exit);