//#include <pic18f4520.h>
#include "config.h"
#include "bits.h"
#include "lcd.h"
#include "keypad.h"
#include "rtc.h"
#include "i2c.h"
#include "atraso.h"
#include "pwm.h"

#define L0 0x80 //Posicionar na linha 0
#define L1 0xC0 //Posicionar na linha 1
#define L2 0x90 //Posicionar na linha 2
#define L3 0xD0 //Posicionar na linha 3
#define CLR 0x01 //Limpar display
#define ON 0x0F //Ativar cursor

unsigned int i, pos = 0, tecla = 16;
unsigned long int cont = 0;
int h_rel, m_rel, s_rel;
int h_alar, m_alar, s_alar;

char msg_principal[2][16] = {"1- Ajustar Alarm", "2- Opcoes Alarm"};
char msg_ajus_alarm[3][16] = {"1 a 6 - Ajustar", "* - Cancelar", "# - Salvar"};
char msg_opc_alarm[4][16] = {"1 - Acend Luzes", "2 - Senha p/ Des", "3 - Soneca", "* - Voltar"};
char msg_luzes[3][16] = {"Luzes: OFF", "1 - Ativ/Desat", "* - Voltar"};
char msg_senha[4][16] = {"Senha: OFF", "1 - Ativ/Desat", "2 - Alter Senha", "* - Voltar"};
char msg_alt_senha[4][16] = {"Atual: ", "Digite: ", "* - Cancelar", "# - Salvar"};
char msg_soneca[4][16] = {"Soneca: OFF", "1 - Ativ/Desat", "2 - Ajust Tempo", "* - Voltar"};
char msg_ajus_tempo[4][16] = {"Tempo:    seg", "1 - Aumentar", "2 - Diminuir", "* - Voltar"};
char msg_alarm_ativ[7][16] = {"* - Desativar", "# - Soneca", "Senha: ", "Senha Incorreta!", "Digite novamente", "Soneca Ativada", "Novo horario:"};

char senha[4], aux_senha[4];
char hor_relo[16], hor_alarm[16] = "00:00:00", aux_alarm[16];
char tempo_son[2] = "05";
int aux_tempo;

void tocarAlarme();
void aciona_buzzer(void);

void main(void) {

    pwmInit();
    TRISC = 0x00;

    lcdInit();
    kpInit();

    for (;;) {
        lcdCommand(CLR);
        //Mostrar Horário Relógio

        lcdCommand(L0);

        for (i = 0; i < 16; i++) {
            lcdData(' ');
        }

        rtc_r();
        for (i = 0; i < 16; i++) {
            hor_relo[i] = time[i];
            lcdData(hor_relo[i]);
        }

        h_rel = ((hor_relo[0] - 48)*10) + (hor_relo[1] - 48);
        m_rel = ((hor_relo[3] - 48)*10) + (hor_relo[4] - 48);
        s_rel = ((hor_relo[6] - 48)*10) + (hor_relo[7] - 48);

        //TEXTO MENU - 1- Ajustar Alarm
        lcdCommand(L2);
        for (i = 0; i < 16; i++) {
            lcdData(msg_principal[0][i]);
        }


        //TEXTO MENU - 2- Opcoes Alarm
        lcdCommand(L3);
        for (i = 0; i < 16; i++) {
            lcdData(msg_principal[1][i]);
        }



        for (;;) {
            //Atualização constante do Relógio
            cont++;

            if ((cont % 180) == 0) {
                s_rel++;
            }

            if (s_rel >= 60) {
                s_rel = 0;
                m_rel++;
                if (m_rel >= 60) {
                    m_rel = 0;
                    h_rel++;
                    if (h_rel >= 24) {
                        h_rel = 0;
                    }
                }
            }

            hor_relo[0] = (h_rel / 10) + 48;
            hor_relo[1] = (h_rel % 10) + 48;
            hor_relo[3] = (m_rel / 10) + 48;
            hor_relo[4] = (m_rel % 10) + 48;
            hor_relo[6] = (s_rel / 10) + 48;
            hor_relo[7] = (s_rel % 10) + 48;

            lcdCommand(L0);
            for (i = 0; i < 8; i++) {
                lcdData(hor_relo[i]);
            }

            //Verificar se algum botão foi pressionado
            kpDebounce();
            if (kpRead() != tecla) {
                tecla = kpRead();
                break;
            }

            //VERIFICAR ALARME 
            for (i = 0; i < 8; i++) {
                if (hor_alarm[i] != hor_relo[i]) {
                    break;
                }
                if (i == 7) {
                    s_rel++;
                    tocarAlarme();
                }
            }
        }

        //Verifica se o botão pressionado foi do ajuste do alarme
        if (bitTst(tecla, 3)) {
            //Guarda horário do alarme em uma variavel auxiliar, caso o usuario cancele
            for (i = 0; i < 8; i++) {
                aux_alarm[i] = hor_alarm[i];
            }

            h_alar = ((hor_alarm[0] - 48)*10) + (hor_alarm[1] - 48);
            m_alar = ((hor_alarm[3] - 48)*10) + (hor_alarm[4] - 48);
            s_alar = ((hor_alarm[6] - 48)*10) + (hor_alarm[7] - 48);

            //Mostrar horário do alarme atual
            lcdCommand(L0);
            for (i = 0; i < 8; i++) {
                lcdData(hor_alarm[i]);
            }

            //Mostrar texto ajuste hora alarme - "1 a 6 - Ajustar"
            lcdCommand(L1);
            for (i = 0; i < 16; i++) {
                lcdData(msg_ajus_alarm[0][i]);
            }

            //Mostrar opção CANCELAR
            lcdCommand(L2);
            for (i = 0; i < 16; i++) {
                lcdData(msg_ajus_alarm[1][i]);
            }

            //Mostrar opção SALVAR
            lcdCommand(L3);
            for (i = 0; i < 16; i++) {
                lcdData(msg_ajus_alarm[2][i]);
            }

            for (;;) {
                kpDebounce();
                if (kpRead() != tecla) {
                    tecla = kpRead();

                    //Incremento - Ajuste dos digitos das Horas
                    if (bitTst(tecla, 3)) {
                        h_alar += 1;

                        if (h_alar > 23)
                            h_alar = 0;

                        hor_alarm[0] = ((h_alar / 10) + 48);
                        hor_alarm[1] = ((h_alar % 10) + 48);
                    }//Decremento - Ajuste dos digitos das Horas
                    else if (bitTst(tecla, 2)) {
                        h_alar -= 1;

                        if (h_alar < 1)
                            h_alar = 23;

                        hor_alarm[0] = ((h_alar / 10) + 48);
                        hor_alarm[1] = ((h_alar % 10) + 48);
                    }//Incremento - Ajuste dos digitos dos Minutos
                    else if (bitTst(tecla, 7)) {
                        m_alar += 1;

                        if (m_alar > 59)
                            m_alar = 0;

                        hor_alarm[3] = ((m_alar / 10) + 48);
                        hor_alarm[4] = ((m_alar % 10) + 48);
                    }//Decremento - Ajuste dos digitos dos Minutos
                    else if (bitTst(tecla, 6)) {
                        m_alar -= 1;

                        if (m_alar < 1)
                            m_alar = 59;

                        hor_alarm[3] = ((m_alar / 10) + 48);
                        hor_alarm[4] = ((m_alar % 10) + 48);
                    }//Incremento - Ajuste dos digitos dos Segundos
                    else if (bitTst(tecla, 11)) {
                        s_alar += 1;

                        if (s_alar > 59)
                            s_alar = 0;

                        hor_alarm[6] = ((s_alar / 10) + 48);
                        hor_alarm[7] = ((s_alar % 10) + 48);
                    }//Decremento - Ajuste dos digitos dos Segundos
                    else if (bitTst(tecla, 10)) {
                        s_alar -= 1;

                        if (s_alar < 1)
                            s_alar = 59;

                        hor_alarm[6] = ((s_alar / 10) + 48);
                        hor_alarm[7] = ((s_alar % 10) + 48);
                    }//Verificar pressionamento da tecla para CANCELAR
                    else if (bitTst(tecla, 0)) {
                        for (i = 0; i < 8; i++) {
                            hor_alarm[i] = aux_alarm[i];
                        }
                        break;
                    }//Verificar pressionamento da tecla para SALVAR
                    else if (bitTst(tecla, 8)) {
                        break;
                    }

                    //Atualiza horario alarme
                    lcdCommand(L0);
                    for (i = 0; i < 8; i++) {
                        lcdData(hor_alarm[i]);
                    }
                }
            }

        }//Verifica se o botão pressionado foi o de opcoes do alarme
        else if (bitTst(tecla, 7)) {
            for (;;) {
                //Mostrar texto "1 - Acend Luzes"
                lcdCommand(L0);
                for (i = 0; i < 16; i++) {
                    lcdData(msg_opc_alarm[0][i]);
                }

                //Mostrar texto "2 - Senha p/ Des"
                lcdCommand(L1);
                for (i = 0; i < 16; i++) {
                    lcdData(msg_opc_alarm[1][i]);
                }

                //Mostrar texto "3 - Soneca"
                lcdCommand(L2);
                for (i = 0; i < 16; i++) {
                    lcdData(msg_opc_alarm[2][i]);
                }

                //Mostrar texto "* - Voltar"
                lcdCommand(L3);
                for (i = 0; i < 16; i++) {
                    lcdData(msg_opc_alarm[3][i]);
                }
                for (;;) {
                    kpDebounce();
                    if (kpRead() != tecla) {
                        tecla = kpRead();
                        break;
                    }
                }

                //Verificar pressionamento da tecla opcao Luzes
                if (bitTst(tecla, 3)) {
                    //TEXTO "Luzes: OFF"
                    lcdCommand(L0);
                    for (i = 0; i < 16; i++) {
                        lcdData(msg_luzes[0][i]);
                    }

                    //Limpar linha L1
                    lcdCommand(L1);
                    for (i = 0; i < 16; i++) {
                        lcdData(' ');
                    }

                    //TEXTO "1 - Ativ/Desat"
                    lcdCommand(L2);
                    for (i = 0; i < 16; i++) {
                        lcdData(msg_luzes[1][i]);
                    }

                    //TEXTO "* - Voltar"
                    lcdCommand(L3);
                    for (i = 0; i < 16; i++) {
                        lcdData(msg_luzes[2][i]);
                    }

                    for (;;) {
                        kpDebounce();
                        if (kpRead() != tecla) {
                            tecla = kpRead();

                            //Verifica se a opcao ativ/desat foi pressionada
                            if (bitTst(tecla, 3)) {
                                if (msg_luzes[0][8] == 'F') {
                                    msg_luzes[0][8] = 'N';
                                    msg_luzes[0][9] = ' ';
                                } else {
                                    msg_luzes[0][8] = 'F';
                                    msg_luzes[0][9] = 'F';
                                }

                                //ATUALIZA TEXTO "1 - Ativ/Desat"
                                lcdCommand(L0);
                                for (i = 0; i < 16; i++) {
                                    lcdData(msg_luzes[0][i]);
                                }
                            } else if (bitTst(tecla, 0)) {
                                break;
                            }
                        }
                    }
                }//Verificar pressionamento da tecla opcao Senha
                else if (bitTst(tecla, 7)) {
                    for (;;) {
                        //TEXTO SENHA
                        lcdCommand(L0);
                        for (i = 0; i < 16; i++) {
                            lcdData(msg_senha[0][i]);
                        }

                        //TEXTO SENHA - "1 - Ativ/Desat"
                        lcdCommand(L1);
                        for (i = 0; i < 16; i++) {
                            lcdData(msg_senha[1][i]);
                        }

                        //TEXTO SENHA - "2 - Alter Senha"
                        lcdCommand(L2);
                        for (i = 0; i < 16; i++) {
                            lcdData(msg_senha[2][i]);
                        }

                        //TEXTO SENHA - "* - Voltar"
                        lcdCommand(L3);
                        for (i = 0; i < 16; i++) {
                            lcdData(msg_senha[3][i]);
                        }

                        for (;;) {
                            kpDebounce();
                            if (kpRead() != tecla) {
                                tecla = kpRead();
                                break;
                            }
                        }

                        if (bitTst(tecla, 3)) {
                            if (msg_senha[0][8] == 'F') {
                                msg_senha[0][8] = 'N';
                                msg_senha[0][9] = ' ';
                            } else {
                                msg_senha[0][8] = 'F';
                                msg_senha[0][9] = 'F';
                            }

                            //ATUALIZA TEXTO "1 - Ativ/Desat"
                            lcdCommand(L0);
                            for (i = 0; i < 16; i++) {
                                lcdData(msg_senha[0][i]);
                            }
                        } else if (bitTst(tecla, 7)) {
                            //TEXTO ALTERAR SENHA - "Atual: "
                            lcdCommand(L0);
                            for (i = 0; i < 7; i++) {
                                lcdData(msg_alt_senha[0][i]);
                            }

                            //MOSTRAR SENHA ATUAL
                            for (i = 0; i < 4; i++) {
                                lcdData(senha[i]);
                            }

                            //TEXTO ALTERAR SENHA - "Digite: "
                            lcdCommand(L1);
                            for (i = 0; i < 16; i++) {
                                lcdData(msg_alt_senha[1][i]);
                            }

                            //TEXTO ALTERAR SENHA - "* - Cancelar"
                            lcdCommand(L2);
                            for (i = 0; i < 16; i++) {
                                lcdData(msg_alt_senha[2][i]);
                            }

                            //TEXTO ALTERAR SENHA - "# - Salvar"
                            lcdCommand(L3);
                            for (i = 0; i < 16; i++) {
                                lcdData(msg_alt_senha[3][i]);
                            }

                            pos = 0;
                            for (i = 0; i < 4; i++) {
                                aux_senha[i] = ' ';
                            }

                            for (;;) {
                                kpDebounce();
                                if (kpRead() != tecla) {
                                    tecla = kpRead();

                                    /*Pegar senha digita pelo usuario*/
                                    if (bitTst(tecla, 4)) {
                                        //Numero0
                                        aux_senha[pos] = '0';
                                        pos++;
                                    } else if (bitTst(tecla, 3)) {
                                        //Numero1
                                        aux_senha[pos] = '1';
                                        pos++;
                                    } else if (bitTst(tecla, 7)) {
                                        //Numero2
                                        aux_senha[pos] = '2';
                                        pos++;
                                    } else if (bitTst(tecla, 11)) {
                                        //Numero3
                                        aux_senha[pos] = '3';
                                        pos++;
                                    } else if (bitTst(tecla, 2)) {
                                        //Numero4
                                        aux_senha[pos] = '4';
                                        pos++;
                                    } else if (bitTst(tecla, 6)) {
                                        //Numero5
                                        aux_senha[pos] = '5';
                                        pos++;
                                    } else if (bitTst(tecla, 10)) {
                                        //Numero6
                                        aux_senha[pos] = '6';
                                        pos++;
                                    } else if (bitTst(tecla, 1)) {
                                        //Numero7
                                        aux_senha[pos] = '7';
                                        pos++;
                                    } else if (bitTst(tecla, 5)) {
                                        //Numero8
                                        aux_senha[pos] = '8';
                                        pos++;
                                    } else if (bitTst(tecla, 9)) {
                                        //Numero9
                                        aux_senha[pos] = '9';
                                        pos++;
                                    } else if (bitTst(tecla, 0)) {
                                        break;
                                    } else if (bitTst(tecla, 8)) {
                                        for (i = 0; i < 4; i++) {
                                            senha[i] = aux_senha[i];
                                        }
                                        break;
                                    }

                                    //Mostrar senha digitada
                                    lcdCommand(L1 + 8);
                                    for (i = 0; i < 4; i++) {
                                        lcdData(aux_senha[i]);
                                    }
                                }
                            }
                        } else if (bitTst(tecla, 0)) {
                            break;
                        }
                    }
                }//Verificar pressionamento da tecla opcao Soneca
                else if (bitTst(tecla, 11)) {
                    for (;;) {
                        //TEXTO SONECA
                        lcdCommand(L0);
                        for (i = 0; i < 16; i++) {
                            lcdData(msg_soneca[0][i]);
                        }

                        //TEXTO SONECA - "1 - Ativ/Desat"
                        lcdCommand(L1);
                        for (i = 0; i < 16; i++) {
                            lcdData(msg_soneca[1][i]);
                        }

                        //TEXTO SONECA - "2 - Ajust Tempo"
                        lcdCommand(L2);
                        for (i = 0; i < 16; i++) {
                            lcdData(msg_soneca[2][i]);
                        }

                        //TEXTO SONECA - "* - Voltar"
                        lcdCommand(L3);
                        for (i = 0; i < 16; i++) {
                            lcdData(msg_soneca[3][i]);
                        }

                        for (;;) {
                            kpDebounce();
                            if (kpRead() != tecla) {
                                tecla = kpRead();
                                break;
                            }
                        }

                        //Verificar pressionamento da opcao ativ/desat soneca
                        if (bitTst(tecla, 3)) {
                            if (msg_soneca[0][9] == 'F') {
                                msg_soneca[0][9] = 'N';
                                msg_soneca[0][10] = ' ';
                            } else {
                                msg_soneca[0][9] = 'F';
                                msg_soneca[0][10] = 'F';
                            }

                            //Atualiza TEXTO SONECA
                            lcdCommand(L0);
                            for (i = 0; i < 16; i++) {
                                lcdData(msg_soneca[0][i]);
                            }

                            //Verificar pressionamento ajuste do tempo da soneca
                        } else if (bitTst(tecla, 7)) {
                            //TEXTO TEMPO SONECA
                            lcdCommand(L0);
                            for (i = 0; i < 16; i++) {
                                lcdData(msg_ajus_tempo[0][i]);
                            }

                            //TEMPO SONECA
                            lcdCommand(L0 + 7);
                            for (i = 0; i < 2; i++) {
                                lcdData(tempo_son[i]);
                            }

                            //TEXTO SONECA - "1 - Aumentar"
                            lcdCommand(L1);
                            for (i = 0; i < 16; i++) {
                                lcdData(msg_ajus_tempo[1][i]);
                            }

                            //TEXTO SONECA - "2 - Diminuir"
                            lcdCommand(L2);
                            for (i = 0; i < 16; i++) {
                                lcdData(msg_ajus_tempo[2][i]);
                            }

                            //TEXTO SONECA - "* - Voltar"
                            lcdCommand(L3);
                            for (i = 0; i < 16; i++) {
                                lcdData(msg_ajus_tempo[3][i]);
                            }

                            //Armazena o tempo de sono em aux como tipo inteiro
                            aux_tempo = ((tempo_son[0] - 48)*10) + (tempo_son[1] - 48);

                            for (;;) {
                                kpDebounce();
                                if (kpRead() != tecla) {
                                    tecla = kpRead();

                                    //Verificar pressionamento opcao aumentar
                                    if (bitTst(tecla, 3)) {
                                        aux_tempo += 5;
                                        if (aux_tempo > 95)
                                            aux_tempo = 5;
                                    }//Verificar pressionamento opcao diminuir
                                    else if (bitTst(tecla, 7)) {
                                        aux_tempo -= 5;
                                        if (aux_tempo < 5)
                                            aux_tempo = 95;
                                    }//Verificar pressionamento opcao voltar
                                    else if (bitTst(tecla, 0)) {
                                        break;
                                    }

                                    //Atualiza tempo da soneca
                                    tempo_son[0] = ((aux_tempo / 10) + 48);
                                    tempo_son[1] = ((aux_tempo % 10) + 48);

                                    //Mostra tempo SONECA
                                    lcdCommand(L0 + 7);
                                    for (i = 0; i < 2; i++) {
                                        lcdData(tempo_son[i]);
                                    }
                                }
                            }
                            //Verificar pressionamento opcao voltar
                        } else if (bitTst(tecla, 0)) {
                            break;
                        }
                    }
                }//Verificar pressionamento da tecla para Voltar
                else if (bitTst(tecla, 0)) {
                    break;
                }
            }
        }
    }
}

void tocarAlarme() {
    char senha_digitada[4];
    int i, cont = 0;
    int tmp;

    TRISA = 0x00;
    TRISD = 0x00;
    PORTD = 0xFF;

    lcdInit();
    kpInit();
    pwmInit();


    lcdCommand(CLR); //Limpar Display

    pwmSet1(96);


    if (msg_luzes[0][8] == 'N') {
        bitSet(PORTA, 5);
        bitSet(PORTA, 4);
        bitSet(PORTA, 3);
        bitSet(PORTA, 2);
    }

    if (msg_senha[0][8] == 'N') {
        for (;;) {
            //Mostrar horario alarme
            lcdCommand(L0);
            for (i = 0; i < 16; i++) {
                lcdData(hor_alarm[i]);
            }

            //Pedir senha para desativar caso a opção esteja ativada
            lcdCommand(L1);
            for (i = 0; i < 7; i++) {
                lcdData(msg_alarm_ativ[2][i]);
            }

            lcdCommand(L2);
            for (i = 0; i < 16; i++) {
                lcdData(msg_alarm_ativ[0][i]);
            }

            lcdCommand(L3);
            for (i = 0; i < 16; i++) {
                lcdData(msg_alarm_ativ[1][i]);
            }

            for (;;) {
                kpDebounce();
                if (kpRead() != tecla) {
                    tecla = kpRead();
                    break;
                }
            }

            if (bitTst(tecla, 4)) {
                //Numero0
                senha_digitada[cont] = '0';
                cont++;
            } else if (bitTst(tecla, 3)) {
                //Numero1
                senha_digitada[cont] = '1';
                cont++;
            } else if (bitTst(tecla, 7)) {
                //Numero2
                senha_digitada[cont] = '2';
                cont++;
            } else if (bitTst(tecla, 11)) {
                //Numero3
                senha_digitada[cont] = '3';
                cont++;
            } else if (bitTst(tecla, 2)) {
                //Numero4
                senha_digitada[cont] = '4';
                cont++;
            } else if (bitTst(tecla, 6)) {
                //Numero5
                senha_digitada[cont] = '5';
                cont++;
            } else if (bitTst(tecla, 10)) {
                //Numero6
                senha_digitada[cont] = '6';
                cont++;
            } else if (bitTst(tecla, 1)) {
                //Numero7
                senha_digitada[cont] = '7';
                cont++;
            } else if (bitTst(tecla, 5)) {
                //Numero8
                senha_digitada[cont] = '8';
                cont++;
            } else if (bitTst(tecla, 9)) {
                //Numero9
                senha_digitada[cont] = '9';
                cont++;
            }

            lcdCommand(L1 + 7);
            for (i = 0; i < 4; i++) {
                lcdData(senha_digitada[i]);
            }

            if (bitTst(tecla, 0)) {
                //botao desativar
                for (i = 0; i < 4; i++) {
                    if (senha[i] != senha_digitada[i]) {
                        cont = 0;
                        break;
                    }
                }

                if (i == 4) {
                    //desativar som
                    break;

                } else {
                    lcdCommand(L1);
                    for (i = 0; i < 16; i++) {
                        lcdData(msg_alarm_ativ[3][i]);
                    }

                    lcdCommand(L2);
                    for (i = 0; i < 16; i++) {
                        lcdData(msg_alarm_ativ[4][i]);
                    }
                    atraso_ms(3000);

                    for (i = 0; i < 4; i++) {
                        senha_digitada[i] = ' ';
                    }

                    lcdCommand(L1);
                    for (i = 0; i < 16; i++) {
                        lcdData(' ');
                    }

                    cont = 0;
                }
            }
            if (bitTst(tecla, 8)) {
                rtc_r();
                tmp = ((time[6] - 48)*10) + (time[7] - 48) + ((tempo_son[0] - 48)*10) + (tempo_son[1] - 48);
                if (tmp > 59) {
                    hor_alarm[4] = ((tmp / 60) + 48);
                    tmp %= 60;
                }

                hor_alarm[6] = ((tmp / 10) + 48);
                hor_alarm[7] = ((tmp % 10) + 48);

                lcdCommand(L0);
                for (i = 0; i < 16; i++) {
                    lcdData(msg_alarm_ativ[5][i]);
                }

                lcdCommand(L1);
                for (i = 0; i < 16; i++) {
                    lcdData(' ');
                }

                lcdCommand(L2);
                for (i = 0; i < 16; i++) {
                    lcdData(msg_alarm_ativ[6][i]);
                }

                lcdCommand(L3);
                for (i = 0; i < 16; i++) {
                    lcdData(hor_alarm[i]);
                }

                atraso_ms(3000);
                break;
            }
        }
    } else {
        //Mostrar horario alarme
        lcdCommand(L0);
        for (i = 0; i < 16; i++) {
            lcdData(hor_alarm[i]);
        }

        //Limpar linha L1
        lcdCommand(L1);
        for (i = 0; i < 16; i++) {
            lcdData(' ');
        }

        //Texto - Desativar
        lcdCommand(L2);
        for (i = 0; i < 16; i++) {
            lcdData(msg_alarm_ativ[0][i]);
        }

        //Texto - Soneca
        lcdCommand(L3);
        for (i = 0; i < 16; i++) {
            lcdData(msg_alarm_ativ[1][i]);
        }

        for (;;) {
            kpDebounce();
            if (kpRead() != tecla) {
                tecla = kpRead();

                //Apagar luzes ao pressionar o botão para desativar
                if (bitTst(tecla, 0)) {
                    // Desativar som
                    break;
                }

                if (bitTst(tecla, 8)) {
                    rtc_r();
                    tmp = ((time[6] - 48)*10) + (time[7] - 48) + ((tempo_son[0] - 48)*10) + (tempo_son[1] - 48);
                    if (tmp > 59) {
                        hor_alarm[4] = ((tmp / 60) + 48);
                        tmp %= 60;
                    }

                    hor_alarm[6] = ((tmp / 10) + 48);
                    hor_alarm[7] = ((tmp % 10) + 48);

                    lcdCommand(L0);
                    for (i = 0; i < 16; i++) {
                        lcdData(msg_alarm_ativ[5][i]);
                    }

                    lcdCommand(L1);
                    for (i = 0; i < 16; i++) {
                        lcdData(' ');
                    }

                    lcdCommand(L2);
                    for (i = 0; i < 16; i++) {
                        lcdData(msg_alarm_ativ[6][i]);
                    }

                    lcdCommand(L3);
                    for (i = 0; i < 16; i++) {
                        lcdData(hor_alarm[i]);
                    }

                    atraso_ms(3000);
                    break;
                }
            }
        }
    }

    pwmSet1(0);
    bitClr(PORTA, 5);
    bitClr(PORTA, 4);
    bitClr(PORTA, 3);
    bitClr(PORTA, 2);
}

void aciona_buzzer(void) {
    pwmFrequency(100);
    for (int i = 0; i < 2; i++) {
        for (int j = 1; j > 0; j = j * 2) {
            bitSet(TRISC, 1);
            atraso_ms(100);
            break;
        }
        bitClr(TRISC, 1);
    }
}

