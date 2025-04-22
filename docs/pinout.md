# Pinagem do Inversor de Frequência Trifásico - STM32F030R8

Este documento descreve todos os pinos utilizados no projeto de inversor de frequência trifásico com controle via porta serial, implementado no microcontrolador STM32F030R8.

## Resumo dos Pinos Utilizados

| Pino | Função | Descrição |
|------|--------|-----------|
| PA2  | USART2_TX | Transmite dados pela porta serial |
| PA3  | USART2_RX | Recebe comandos pela porta serial |
| PA5  | LED de Status | Indica estado do sistema e pisca em caso de erro |
| PA8  | TIM1_CH1 | Saída PWM para fase U do inversor trifásico |
| PA9  | TIM1_CH2 | Saída PWM para fase V do inversor trifásico |
| PA10 | TIM1_CH3 | Saída PWM para fase W do inversor trifásico |

## Detalhes de Configuração

### Comunicação Serial (USART2)
- **Pinos**: PA2 (TX) e PA3 (RX)
- **Configuração**: 
  - Modo: Função alternativa (GPIO_MODE_AF_PP)
  - Função alternativa: GPIO_AF1_USART2
  - Resistor de pull-up/down: Nenhum (GPIO_NOPULL)
  - Velocidade: Alta (GPIO_SPEED_FREQ_HIGH)
- **Parâmetros da UART**: 
  - Baud rate: 115200 bps
  - 8 bits de dados
  - Sem paridade
  - 1 bit de parada
  - Sem controle de fluxo (hardware)
  - Oversampling 16x

### Saídas PWM (Timer 1)
- **Pinos**: PA8 (TIM1_CH1), PA9 (TIM1_CH2), PA10 (TIM1_CH3)
- **Configuração**: 
  - Modo: Função alternativa (GPIO_MODE_AF_PP)
  - Função alternativa: GPIO_AF2_TIM1
  - Resistor de pull-up/down: Nenhum (GPIO_NOPULL)
  - Velocidade: Alta (GPIO_SPEED_FREQ_HIGH)
- **Parâmetros do Timer**:
  - Modo de contagem: Centro-alinhado (TIM_COUNTERMODE_CENTERALIGNED1)
  - Período: 1000 (PWM carrier frequency = TIM1CLK/(2*Period))
  - Dead time: 100 ticks (para evitar curto-circuito entre transistores superiores e inferiores)
  - Sem prescaler (Prescaler = 0)
  - Frequência da portadora PWM: ~24 kHz (com clock de 48 MHz)

### LED de Status (PA5)
- **Pino**: PA5
- **Configuração**:
  - Modo: Saída push-pull (GPIO_MODE_OUTPUT_PP)
  - Resistor de pull-up/down: Nenhum (GPIO_NOPULL)
  - Velocidade: Baixa (GPIO_SPEED_FREQ_LOW)
- **Função**: O LED pisca a cada 200ms em caso de erro no sistema

## Mapa de Conexões Recomendado

```
                      +-------------------+
                      |                   |
                      |    STM32F030R8    |
                      |                   |
                      |           PA2(TX) +--------> PC/Terminal (RX)
                      |                   |
                      |           PA3(RX) +<-------- PC/Terminal (TX)
                      |                   |
                      |                   |          +-------------+
                      |          PA8(CH1) +--------> | Driver Fase U |---> Fase U
                      |                   |          +-------------+
                      |                   |          +-------------+
                      |          PA9(CH2) +--------> | Driver Fase V |---> Fase V
                      |                   |          +-------------+
                      |                   |          +-------------+
                      |         PA10(CH3) +--------> | Driver Fase W |---> Fase W
                      |                   |          +-------------+
                      |                   |
                      |            PA5    +--------> LED de Status
                      |                   |
                      +-------------------+
```

## Observações Importantes para Implementação do Hardware

1. **Isolação**: É fortemente recomendado usar optoacopladores ou isoladores digitais entre os pinos PWM do microcontrolador e os drivers de potência. Isso protege o microcontrolador de transientes e ruídos da parte de potência.

2. **Drivers para MOSFETs/IGBTs**: Os sinais PWM gerados pelos pinos PA8, PA9 e PA10 necessitam de drivers adequados para acionar os transistores de potência. Para potências maiores (acima de 5kW), recomenda-se o uso de drivers robustos como SCALE-2 da Power Integrations (2SC0435T, 2SP0115T), Infineon EiceDRIVER, ou Semikron SKYPER. Para potências intermediárias (até 5kW), drivers como IR2110/IR2113, IR2213, ou módulos integrados como o 6EDL04I06PT são adequados.

3. **Conversor USB-Serial**: Para conectar o inversor ao computador, é necessário um conversor USB-Serial (como FTDI FT232, CP2102, CH340) conectado aos pinos PA2 e PA3.

4. **Proteção de Sobrecorrente**: Recomenda-se adicionar sensores de corrente e circuitos de proteção para evitar danos aos transistores de potência em caso de sobrecarga ou curto-circuito.

5. **Fonte de Alimentação**: O circuito requer uma fonte isolada para o microcontrolador (3.3V) e fontes adequadas para os drivers e etapa de potência.

6. **Dissipadores de Calor**: Os MOSFETs/IGBTs devem ser montados em dissipadores de calor adequados para evitar superaquecimento.

## Comandos Disponíveis via Porta Serial

- `FREQ <valor>` - Define a frequência de saída do inversor (0.1-50.0 Hz)
- `START` - Inicia o inversor
- `STOP` - Para o inversor
- `STATUS` - Mostra o estado atual e a frequência configurada
- `HELP` - Exibe todos os comandos disponíveis
