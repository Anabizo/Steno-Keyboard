# Stenography Kernel Module (AnaType)

## 📝 Descrição

O **AnaType** é um módulo de kernel Linux que implementa um sistema de estenotipia em tempo real, interceptando eventos de teclado e expandindo abreviações automaticamente. O projeto oferece uma solução inovadora para aumentar a produtividade na digitação através de expansão inteligente de texto com capitalização automática, sem precisar de equipamentos específicos para o fim.

## 🎯 Funcionalidades

- **Interceptação de Teclado**: Captura eventos de teclado em tempo real no nível do kernel
- **Expansão de Abreviações**: Sistema de dicionário configurável para expansão de texto
- **Capitalização Inteligente**: Automaticamente capitaliza palavras no início de frases
- **Ordenação de Caracteres**: Permite digitar abreviações em qualquer ordem
- **Dispositivo Virtual**: Injeta texto expandido através de dispositivo virtual
- **Buffer Inteligente**: Gerenciamento de buffer com backspace funcional

## 🚀 Como Funciona

1. **Captura**: O módulo intercepta teclas digitadas pelo usuário
2. **Armazenamento**: Caracteres são armazenados em buffer interno
3. **Processamento**: Ao pressionar espaço, busca a palavra no dicionário
4. **Expansão**: Se encontrada, injeta a palavra completa; senão, injeta a original

### Exemplo de Uso
```
Digite: "mn cs de in" + espaços
Resultado: "Minha casa de inverno"

Digite: "pq n" + espaços  
Resultado: "porque nao"
```

## 🔧 Compilação e Instalação

```bash
# Compilar o módulo
make

# Carregar o módulo
make install

# Ver logs em tempo real
make test

# Descarregar o módulo
make remove
```

## 📋 Requisitos do Sistema

- Linux Kernel 5.0+
- Headers do kernel instalados
- Permissões de administrador

## 🏗️ Arquitetura Técnica

### Visão Geral do Sistema

<img width="3840" height="3606" alt="Untitled diagram _ Mermaid Chart-2025-08-28-012900" src="https://github.com/user-attachments/assets/03d0dee4-a51f-4935-ae07-441b204ca6ab" />

O sistema opera em múltiplas camadas do kernel Linux, desde o hardware até as aplicações de usuário:

#### **Kernel Space**
- **Módulo Stenográfico**: Núcleo do sistema contendo todos os componentes principais
- **Input Handler**: Intercepta eventos diretamente do teclado físico
- **Buffer Interno**: Armazena caracteres temporariamente (max 64 chars)
- **Dicionário**: Mapeia abreviações para palavras completas
- **String Injector**: Injeta texto expandido no sistema
- **Dispositivo Virtual**: Interface para comunicação com aplicações

#### **Input Subsystem**
- **Input Events Handler**: Gerencia distribuição de eventos para aplicações
- Conecta dispositivos físicos às aplicações do usuário

#### **User Space**
- **Editor de Texto**: Recebe texto expandido automaticamente
- **Terminal**: Interface de linha de comando
- **Navegador**: Aplicações web com suporte completo

### Fluxo de Processamento Detalhado

<img width="3840" height="3688" alt="Untitled diagram _ Mermaid Chart-2025-08-28-012350" src="https://github.com/user-attachments/assets/f310b4c8-66cb-40d2-9cf9-a5c843c7ada3" />

O processamento segue um fluxo inteligente baseado no tipo de tecla pressionada:

#### **1. Captura de Eventos**
- Sistema detecta quando usuário pressiona qualquer tecla
- Determina o tipo: letra, espaço, backspace ou enter

#### **2. Processamento de Letras (a-z)**
- Adiciona caractere ao buffer interno
- Bloqueia tecla original para evitar duplicação
- Aguarda próxima ação do usuário

#### **3. Processamento de Espaço**
- **Com Buffer Vazio**: Injeta espaço normal
- **Com Buffer Preenchido**:
  - Busca palavra no dicionário (com ordenação de caracteres)
  - **Encontrou**: Injeta palavra expandida + espaço
  - **Não Encontrou**: Injeta palavra original + espaço
  - Limpa buffer após processamento

#### **4. Processamento de Backspace**
- **Com Buffer Vazio**: Permite backspace normal
- **Com Buffer Preenchido**: Remove último caractere do buffer e bloqueia tecla

#### **5. Processamento de Enter**
- Limpa buffer automaticamente
- Permite enter normal para quebra de linha

### Componentes Principais

1. **Input Handler**: Intercepta eventos de teclado via subsistema input do Linux
2. **Buffer Manager**: Gerencia armazenamento temporário de caracteres
3. **Dictionary Engine**: Sistema de busca com ordenação de caracteres
4. **Virtual Device**: Dispositivo de entrada virtual para injeção de texto
5. **Capitalization Engine**: Sistema inteligente de capitalização

### Estruturas de Dados

```c
struct steno_entry {
    char abbreviation[16];
    char expansion[MAX_WORD_LENGTH];
};

struct kb_handle {
    struct input_handle ih;
    bool processing;
    char buffer[MAX_BUFFER_SIZE + 1];
    int buffer_pos;
};
```

## Demonstração
https://github.com/user-attachments/assets/f0627f23-5555-4e51-9458-2e14bfa6206d

## 📊 Atendimento aos Requisitos Obrigatórios

### ✅ Interface Direta com Hardware
- **Implementado**: Interceptação direta de eventos de teclado via subsistema input do kernel Linux
- **Detalhes**: O módulo registra um handler que captura eventos EV_KEY diretamente dos dispositivos de entrada
- **Código**: Função `swap_filter()` processa eventos em tempo real

### ✅ Gerenciamento de Dispositivo Não Suportado
- **Implementado**: Criação e gerenciamento de dispositivo virtual de teclado
- **Detalhes**: O módulo cria um dispositivo virtual (`/dev/input/eventX`) para injeção de texto
- **Código**: Funções `create_virtual_device()` e `destroy_virtual_device()`

### ✅ Técnicas de Depuração e Prototipação Virtual
- **Implementado**: Sistema completo de logging e depuração via `pr_info()`, `pr_debug()`, `pr_warn()`
- **Detalhes**: Logs detalhados de cada operação, desde captura até injeção de texto
- **Prototipação**: Desenvolvimento incremental com testes em máquina virtual

### ✅ Planejamento e Versionamento
- **Implementado**: Controle de versão Git com commits estruturados
- **Detalhes**: Desenvolvimento organizado com metodologia Kanban usando o projeto do GitHub
- **Repositório**: Histórico completo de desenvolvimento disponível

## 🎯 Atendimento ao Requisito Desejável

### ✅ Potencial Comercial e Transferência de Tecnologia

#### 💼 **Aplicação Comercial**
- **Mercado Alvo**: Profissionais que fazem digitação intensiva (jornalistas, escritores, programadores, assistentes jurídicos)
- **Modelo de Negócio**: Licenciamento de software empresarial
- **Diferencial**: Sistema de estenotipia que não precisa de teclado próprio para utilizar

#### 🏭 **Transferência de Tecnologia**
- **Código Aberto**: Base tecnológica disponível para adaptação
- **Modularidade**: Sistema pode ser adaptado para outros idiomas e casos de uso

## 🔮 Roadmap Futuro

- [ ] Suporte para acentuação
- [ ] Interface gráfica para configuração
- [ ] Suporte a múltiplos idiomas
- [ ] Aprendizado automático de padrões
- [ ] Integração com editores populares
- [ ] Aplicativo mobile
- [ ] Dashboard de produtividade

## 🤝 Contribuindo

Este projeto representa uma inovação significativa na interação humano-computador, oferecendo uma solução robusta e comercialmente viável para otimização de produtividade em digitação.

## 📜 Licença

GPL - Compatível com kernel Linux

---

**Desenvolvido por**: Ana Bizo  
**Tecnologia**: Kernel Linux, C, Input Subsystem  
**Potencial Comercial**: Alto ⭐⭐⭐⭐⭐
