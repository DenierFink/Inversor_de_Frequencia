# Inversor de Frequência Trifásico com Controle Serial

Projeto de um inversor de frequência trifásico controlado via interface serial, desenvolvido para o microcontrolador STM32F030R8.

## Características

- **Microcontrolador**: STM32F030R8 (Discovery)
- **Controle**: Via interface serial (115200 bps, 8N1)
- **Saída**: Três fases PWM com frequência ajustável (0.1Hz - 50Hz)
- **Frequência da portadora PWM**: ~24 kHz
- **Dead time**: Configurável (100 ticks por padrão)

## Estrutura do Projeto

- `src/`: Código fonte em C
  - `main.c`: Ponto de entrada do programa
  - `freq_control.c`: Controle de frequência do inversor
  - `pwm_control.c`: Geração dos sinais PWM
  - `serial_comm.c`: Interface de comunicação serial
- `docs/`: Documentação
  - `pinout.md`: Descrição detalhada dos pinos utilizados

## Comandos Disponíveis

O controle do inversor é feito por meio de comandos enviados pela porta serial:

- `FREQ <valor>`: Define a frequência de saída (0.1-50.0 Hz)
- `START`: Inicia o inversor
- `STOP`: Para o inversor
- `STATUS`: Mostra o estado atual e a frequência configurada
- `HELP`: Exibe os comandos disponíveis

## Hardware Sugerido

Para a implementação completa, são necessários componentes externos:
- Drivers para MOSFETs/IGBTs (IR2110/IR2113 para potências até 5kW)
- Para potências acima de 5kW, drivers como SCALE-2, EiceDRIVER ou SKYPER
- Circuitos de isolação óptica/digital
- Sensores de corrente para proteção
- Conversor USB-Serial para comunicação com computador

## Instalação e Compilação

Este projeto utiliza o PlatformIO como ambiente de desenvolvimento:

1. Clone o repositório:
   ```
   git clone https://github.com/seuusuario/Inversor_de_Frequencia.git
   ```

2. Abra o projeto no VSCode com a extensão PlatformIO

3. Compile o projeto:
   ```
   pio run
   ```

4. Faça o upload para o microcontrolador:
   ```
   pio run --target upload
   ```

## Licença

[Especifique a licença do projeto]

## Autor

[Seu nome/organização]
