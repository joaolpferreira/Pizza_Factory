#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BLUE "[0,191,205]"
#define GREEN "[0,254,0]"
#define RED "[254,0,0]"
#define BLACK "[0,0,0]"
#define GRAY "[105,105,105]"

struct regra
{
    char sensor1[5];
    char sensor2[5];
    char sensor3[5];
    char sensor4[5]; //limite 4 sensores por regra
    char comp1[2];
    char comp2[2];
    char comp3[2];
    char comp4[2];
    int valor1;
    int valor2;
    int valor3;
    int valor4;
    char atuador1[5];
    char atuador2[5];
    char atuador3[5];
    char atuador4[5];
    char logico1[4];
    char logico2[4];
    char logico3[4];
    int estado1;
    int estado2;
    int estado3;
    int estado4;
    int ativa;

};

struct config
{
    char sensores[150];
   
    char at1[5];
    char at2[5];
    char at3[5];
    char at4[5];
    int est1;
    int est2;
    int est3;
    int est4;
    char nome[150];
};

struct mote
{
    struct regra regras[9];
    struct config configs;
};

void guarda_sensor(struct regra *reg, char *aux, int ident);
void mote_init(struct mote *mot);
void guarda_atuadores(struct regra *reg, char *aux, int ident);
int hex_to_decimal(char byte[]);
float voltage (char mensagem[]);
float temperature (char mensagem[]);
float relative_humidity(char mensagem[]);
float humidity_compensated_temperature(char hum[], char temp[], float lin);
float visible_light (char mensagem[]);
float current (char mensagem[]);
int verifica_regras(struct regra *reg, int num, char *raw_temperature, char *raw_humidity, char *raw_visible_light, char *raw_voltage, char *raw_current);
void Print_Matrix(int LED_FAC, int A_C, int DEHUMI, int CONV_BELT, int LED_OVEN, int OVEN, int A_C_3, int DEHUMI_3);
void linearidade(int caso);

int main()
{
    FILE *fp, *sensor_rules, *sensor_configs;
    int i=0, j=0, k, y, cont=0, cont1=0, cont2=0, cursor , calc_logico=0, old_estado1=0, atua1=0, atua2=0, atua3=0, atua4=0, atua5=0, auxiliar[7]={0,0,0,0,0,0}, auxiliar1[7]={0,0,0,0,0,0}, auxiliar2[7]={0,0,0,0,0,0};
    char str[150], *bytes, mote_id[5], raw_voltage[5], raw_visible_light[5], raw_current[5], raw_temperature[5], raw_humidity[5], nome[150], sensores[150], atuadores[200], *aux_sensor, *aux_atuador, at_aux[5], *test[24];
    struct mote mote1;
    struct mote mote2;
    struct mote mote3; //MAXIMO 3 MOTES
    
    mote_init(&mote1);
    mote_init(&mote2);
    mote2.configs.est2=1;
    mote_init(&mote3);
    

    sensor_configs = fopen("SensorConfigurations.txt", "r");

    if (sensor_configs == NULL)
    {
        printf("ERRO\n");
        return (-1);
    }
    
    while (1)
    {
   
        if( fscanf(sensor_configs, "%[^:]:%[^:]:%s\n", mote1.configs.nome, mote1.configs.sensores, at_aux) !=EOF)
        {
        strcpy(mote1.configs.at1,strtok(at_aux, ","));
        strcpy(mote1.configs.at2, strtok(NULL, ","));
        strcpy(mote1.configs.at3, strtok(NULL, ","));
        strcpy(mote1.configs.at4, strtok(NULL, "\0"));
        }

        if(fscanf(sensor_configs, "%[^:]:%[^:]:%s\n", mote2.configs.nome, mote2.configs.sensores, at_aux)!= EOF)
        {
        strcpy(mote2.configs.at1,strtok(at_aux, ","));
        strcpy(mote2.configs.at2, strtok(NULL, "\0"));
        }
        if(fscanf(sensor_configs, "%[^:]:%[^:]:%s\n", mote3.configs.nome, mote3.configs.sensores, at_aux)!= EOF)
        {
        strcpy(mote3.configs.at1,strtok(at_aux, ","));
        strcpy(mote3.configs.at2, strtok(NULL, "\0"));
        }

        break;
    }


    fclose(sensor_configs);
    
    sensor_rules = fopen("SensorRules.txt", "r");
    if(sensor_rules == NULL)
    {
        printf("ERRO NO FICHEIRO\n");
        return -1;
    }

    while(1)
    {   
        
        cursor =fscanf(sensor_rules, "%[^:]:%[^:]:%s\n", nome, sensores, atuadores);
       
        if(cursor == EOF)
        {
            break;
        } 
       
        //SENSORES
       
        
        if(strcmp(mote1.configs.nome, nome)==0)
        {
            aux_sensor = strtok(sensores, " ");
            guarda_sensor(&mote1.regras[cont], aux_sensor, 1);

            aux_sensor = strtok(NULL, " ");
            if(aux_sensor != NULL)
            {
                strncpy(mote1.regras[cont].logico1, aux_sensor, strlen(aux_sensor));
                mote1.regras[cont].logico1[strlen(aux_sensor)]='\0';
                aux_sensor = strtok(NULL, " ");
                guarda_sensor(&mote1.regras[cont], aux_sensor, 2);
            
                aux_sensor = strtok(NULL, " ");
                if(aux_sensor != NULL)
                {
                    strncpy(mote1.regras[cont].logico2, aux_sensor, strlen(aux_sensor));
                    mote1.regras[cont].logico2[strlen(aux_sensor)]='\0';
                    aux_sensor = strtok(NULL, " ");
                    guarda_sensor(&mote1.regras[cont], aux_sensor, 3);
                    aux_sensor = strtok(NULL, " ");
                    if(aux_sensor != NULL)
                    {
                        strncpy(mote1.regras[cont].logico3, aux_sensor, strlen(aux_sensor));
                        mote1.regras[cont].logico3[strlen(aux_sensor)]='\0';
                        aux_sensor = strtok(NULL, " ");
                        guarda_sensor(&mote1.regras[cont], aux_sensor, 4);
                    }
                    
                }
                
            }
            aux_atuador = strtok(atuadores, ",");
            guarda_atuadores(&mote1.regras[cont], aux_atuador, 1);
            aux_atuador = strtok(NULL, ",");
            
            if(aux_atuador !=NULL)
            {
                guarda_atuadores(&mote1.regras[cont], aux_atuador, 2);
                aux_atuador = strtok(NULL, ",");
                if(aux_atuador !=NULL)
                {
                    guarda_atuadores(&mote1.regras[cont], aux_atuador, 3);
                    aux_atuador = strtok(NULL, ",");
                    if(aux_atuador !=NULL)
                    {
                        guarda_atuadores(&mote1.regras[cont], aux_atuador, 4);
                    }
                }
            }


            cont++; 
        }
        if(strcmp(mote2.configs.nome, nome)==0)
        {
            aux_sensor = strtok(sensores, " ");
            guarda_sensor(&mote2.regras[cont1], aux_sensor, 1);
            

            aux_sensor = strtok(NULL, " ");
           
            if(aux_sensor != NULL)
            {
               
                strncpy(mote2.regras[cont1].logico1, aux_sensor, strlen(aux_sensor));
                mote2.regras[cont1].logico1[strlen(aux_sensor)]='\0';
                aux_sensor = strtok(NULL, " ");
                guarda_sensor(&mote2.regras[cont1], aux_sensor, 2);
               
                aux_sensor = strtok(NULL, " ");
                if(aux_sensor != NULL)
                {
                    strncpy(mote2.regras[cont1].logico2, aux_sensor, strlen(aux_sensor));
                    mote2.regras[cont1].logico2[strlen(aux_sensor)]='\0';
                    aux_sensor = strtok(NULL, " ");
                    guarda_sensor(&mote2.regras[cont1], aux_sensor, 3);
                    aux_sensor = strtok(NULL, " ");
                    if(aux_sensor != NULL)
                    {
                        strncpy(mote2.regras[cont1].logico3, aux_sensor, strlen(aux_sensor));
                        mote2.regras[cont1].logico3[strlen(aux_sensor)]='\0';
                        aux_sensor = strtok(NULL, " ");
                        guarda_sensor(&mote2.regras[cont1], aux_sensor, 4);
                    }
                }
            }
            aux_atuador = strtok(atuadores, ",");
            guarda_atuadores(&mote2.regras[cont1], aux_atuador, 1);
            
            aux_atuador = strtok(NULL, ",");
            
            
            if(aux_atuador !=NULL)
            {
                guarda_atuadores(&mote2.regras[cont1], aux_atuador, 2);
                aux_atuador = strtok(NULL, ",");
                if(aux_atuador !=NULL)
                {
                    guarda_atuadores(&mote2.regras[cont1], aux_atuador, 3);
                    aux_atuador = strtok(NULL, ",");
                    if(aux_atuador !=NULL)
                    {
                        guarda_atuadores(&mote2.regras[cont1], aux_atuador, 4);
                    }
                }
            }   
            cont1++;
        }
        if(strcmp(mote3.configs.nome, nome)==0)
        {
            aux_sensor = strtok(sensores, " ");
            guarda_sensor(&mote3.regras[cont2], aux_sensor, 1);
            

            aux_sensor = strtok(NULL, " ");
           
            if(aux_sensor != NULL)
            {
               
                strncpy(mote3.regras[cont2].logico1, aux_sensor, strlen(aux_sensor));
                mote3.regras[cont2].logico1[strlen(aux_sensor)]='\0';
                aux_sensor = strtok(NULL, " ");
                guarda_sensor(&mote3.regras[cont2], aux_sensor, 2);
               
                aux_sensor = strtok(NULL, " ");
                if(aux_sensor != NULL)
                {
                    strncpy(mote3.regras[cont2].logico2, aux_sensor, strlen(aux_sensor));
                    mote3.regras[cont2].logico2[strlen(aux_sensor)]='\0';
                    aux_sensor = strtok(NULL, " ");
                    guarda_sensor(&mote3.regras[cont2], aux_sensor, 3);
                    aux_sensor = strtok(NULL, " ");
                    if(aux_sensor != NULL)
                    {
                        strncpy(mote3.regras[cont2].logico3, aux_sensor, strlen(aux_sensor));
                        mote3.regras[cont2].logico3[strlen(aux_sensor)]='\0';
                        aux_sensor = strtok(NULL, " ");
                        guarda_sensor(&mote3.regras[cont2], aux_sensor, 4);
                    }
                }
            }
            aux_atuador = strtok(atuadores, ",");
            guarda_atuadores(&mote3.regras[cont2], aux_atuador, 1);
            
            aux_atuador = strtok(NULL, ",");
            
            
            if(aux_atuador !=NULL)
            {
                guarda_atuadores(&mote3.regras[cont2], aux_atuador, 2);
                aux_atuador = strtok(NULL, ",");
                if(aux_atuador !=NULL)
                {
                    guarda_atuadores(&mote3.regras[cont2], aux_atuador, 3);
                    aux_atuador = strtok(NULL, ",");
                    if(aux_atuador !=NULL)
                    {
                        guarda_atuadores(&mote3.regras[cont2], aux_atuador, 4);
                    }
                }
            }
            cont2++;
        }


    }
    fclose(sensor_rules);
   
 


    while(1)
    {
        fp =fopen("/tmp/ttyV11", "r+");
        if (fp == NULL) 
        {
            printf("ERRO\n");
            return(-1);
        }
        
        if(fgets(str, 150, fp) == NULL) break;

     
        i++;
        if(i==1) 
        {
            
            continue;
            
        }

        bytes = strtok(str, " ");
        j=0;
        while(bytes != '\0')
        {    
            test[j]=bytes;  
           
    
            bytes = strtok(NULL, " ");
           
            j++;  
        }
        
        
        
        strncpy(mote_id, strcat(test[5], test[6]), 4);
        mote_id[4]='\0';
        strncpy(raw_voltage, strcat(test[10], test[11]), 4);
        raw_voltage[4]='\0';
        strncpy(raw_visible_light, strcat(test[12], test[13]), 4); 
        raw_visible_light[4]='\0'; 
        strncpy(raw_current, strcat(test[14], test[15]), 4);
        raw_current[4]='\0';
        strncpy(raw_temperature, strcat(test[16], test[17]), 4);
        raw_temperature[4]='\0';
        strncpy(raw_humidity, strcat(test[18], test[19]), 4);
        raw_humidity[4]='\0';

    
       

       //MOTE_ID

        if(hex_to_decimal(mote_id)==1)
        {
            for(k=0;k<8;++k)
            {
               
                calc_logico=verifica_regras(&mote1.regras[k], 1, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current);
                
                if((strcmp(mote1.regras[k].logico1, "\0")!=0) && calc_logico==1)
                {
                    if (strcmp(mote1.regras[k].logico1, "AND")==0)
                    {
                        
                       
                        calc_logico= (calc_logico && verifica_regras(&mote1.regras[k], 2, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                        
                        if((strcmp(mote1.regras[k].logico2, "\0")!=0) && calc_logico==1)
                        {
                            if (strcmp(mote1.regras[k].logico2, "AND")==0)
                            {
                                calc_logico= (calc_logico && verifica_regras(&mote1.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));

                                if((strcmp(mote1.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote1.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote1.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote1.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    
                                }
                            }
                            else
                            {
                                calc_logico= (calc_logico || verifica_regras(&mote1.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                if((strcmp(mote1.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote1.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote1.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote1.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        calc_logico= (calc_logico || verifica_regras(&mote1.regras[k], 2, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                        if((strcmp(mote1.regras[k].logico2, "\0")!=0) && calc_logico==1)
                        {
                            if (strcmp(mote1.regras[k].logico2, "AND")==0)
                            {
                                calc_logico= (calc_logico && verifica_regras(&mote1.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                if((strcmp(mote1.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote1.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote1.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote1.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                }

                            }
                            else
                            {
                                calc_logico= (calc_logico || verifica_regras(&mote1.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                if((strcmp(mote1.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote1.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote1.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote1.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                }
                            }
                        }

                    }
    
                }
                
                if (calc_logico==1)
                {   
                    mote1.regras[k].ativa=1;
                }
                
                if(calc_logico == 0)
                {
                    mote1.regras[k].ativa=0;
                }
                
        
              
                if((mote1.regras[k].ativa==1 && auxiliar[k]==0) )
                {
                    

                    if(strlen(mote1.regras[k].atuador1)!=0) 
                    {
                        if(strcmp(mote1.regras[k].atuador1, mote1.configs.at1)==0) mote1.configs.est1=mote1.regras[k].estado1;
                        if(strcmp(mote1.regras[k].atuador1, mote1.configs.at2)==0) mote1.configs.est2=mote1.regras[k].estado1;
                        if(strcmp(mote1.regras[k].atuador1, mote1.configs.at3)==0) mote1.configs.est3=mote1.regras[k].estado1;
                        if(strcmp(mote1.regras[k].atuador1, mote1.configs.at4)==0) mote1.configs.est4=mote1.regras[k].estado1;
                    }
                    if(strlen(mote1.regras[k].atuador2)!=0) 
                    {
                        if(strcmp(mote1.regras[k].atuador2, mote1.configs.at1)==0) mote1.configs.est1=mote1.regras[k].estado2;
                        if(strcmp(mote1.regras[k].atuador2, mote1.configs.at2)==0) mote1.configs.est2=mote1.regras[k].estado2;
                        if(strcmp(mote1.regras[k].atuador2, mote1.configs.at3)==0) mote1.configs.est3=mote1.regras[k].estado2;
                        if(strcmp(mote1.regras[k].atuador2, mote1.configs.at4)==0) mote1.configs.est4=mote1.regras[k].estado2;
                    }
                    if(strlen(mote1.regras[k].atuador3)!=0)
                    {
                        if(strcmp(mote1.regras[k].atuador3, mote1.configs.at1)==0) mote1.configs.est1=mote1.regras[k].estado3;
                        if(strcmp(mote1.regras[k].atuador3, mote1.configs.at2)==0) mote1.configs.est2=mote1.regras[k].estado3;
                        if(strcmp(mote1.regras[k].atuador3, mote1.configs.at3)==0) mote1.configs.est3=mote1.regras[k].estado3;
                        if(strcmp(mote1.regras[k].atuador3, mote1.configs.at4)==0) mote1.configs.est4=mote1.regras[k].estado3;
                    }
                    if(strlen(mote1.regras[k].atuador4)!=0)
                    {
                        if(strcmp(mote1.regras[k].atuador4, mote1.configs.at1)==0) mote1.configs.est1=mote1.regras[k].estado4;
                        if(strcmp(mote1.regras[k].atuador4, mote1.configs.at2)==0) mote1.configs.est2=mote1.regras[k].estado4;
                        if(strcmp(mote1.regras[k].atuador4, mote1.configs.at3)==0) mote1.configs.est3=mote1.regras[k].estado4;
                        if(strcmp(mote1.regras[k].atuador4, mote1.configs.at4)==0) mote1.configs.est4=mote1.regras[k].estado4;
                    }

                    if(strcmp(mote1.regras[k].sensor1, "Tem1")==0) linearidade(1);
                    if(strcmp(mote1.regras[k].sensor1, "Hum1")==0) linearidade(2);
                    if(strcmp(mote1.regras[k].sensor2, "Tem1")==0) linearidade(1);
                    if(strcmp(mote1.regras[k].sensor2, "Hum1")==0) linearidade(2);
                    if(strcmp(mote1.regras[k].sensor3, "Tem1")==0) linearidade(1);
                    if(strcmp(mote1.regras[k].sensor3, "Hum1")==0) linearidade(2);
                    if(strcmp(mote1.regras[k].sensor4, "Tem1")==0) linearidade(1);
                    if(strcmp(mote1.regras[k].sensor4, "Hum1")==0) linearidade(2);
                }

                
              
                auxiliar[k]=mote1.regras[k].ativa;
               
               
                
                
               
            }
          
            
        }
        
        if(hex_to_decimal(mote_id)==2)
        {
            
            for(k=0;k<8;++k)
            {
              
                calc_logico=verifica_regras(&mote2.regras[k], 1, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current);
                
                if((strcmp(mote2.regras[k].logico1, "\0")!=0) && calc_logico==1)
                {
                    if (strcmp(mote2.regras[k].logico1, "AND")==0)
                    {
                        calc_logico= (calc_logico && verifica_regras(&mote2.regras[k], 2, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                       
                        if((strcmp(mote2.regras[k].logico2, "\0")!=0) && calc_logico==1)
                        {
                            if (strcmp(mote2.regras[k].logico2, "AND")==0)
                            {
                                calc_logico= (calc_logico && verifica_regras(&mote2.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));

                                if((strcmp(mote2.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote2.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote2.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote2.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    
                                }
                            }
                            else
                            {
                                calc_logico= (calc_logico || verifica_regras(&mote2.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                if((strcmp(mote2.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote2.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote2.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote2.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        calc_logico= (calc_logico || verifica_regras(&mote2.regras[k], 2, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                        if((strcmp(mote2.regras[k].logico2, "\0")!=0) && calc_logico==1)
                        {
                            if (strcmp(mote2.regras[k].logico2, "AND")==0)
                            {
                                calc_logico= (calc_logico && verifica_regras(&mote2.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                if((strcmp(mote2.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote2.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote2.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote2.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                }

                            }
                            else
                            {
                                calc_logico= (calc_logico || verifica_regras(&mote2.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                if((strcmp(mote2.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote2.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote2.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote2.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                }
                            }
                        }

                    }
    
                }
             
                if (calc_logico==1)
                {   
                    mote2.regras[k].ativa=1;
                }
                if(calc_logico == 0)
                {
                    mote2.regras[k].ativa=0;
                }
                
               
               
                if((mote2.regras[k].ativa==1 && auxiliar1[k]==0) )
                {
                    if(strlen(mote2.regras[k].atuador1)!=0) 
                    {
                        if(strcmp(mote2.regras[k].atuador1, mote2.configs.at1)==0) mote2.configs.est1=mote2.regras[k].estado1;
                        if(strcmp(mote2.regras[k].atuador1, mote2.configs.at2)==0) mote2.configs.est2=mote2.regras[k].estado1;
                        if(strcmp(mote2.regras[k].atuador1, mote2.configs.at3)==0) mote2.configs.est3=mote2.regras[k].estado1;
                        if(strcmp(mote2.regras[k].atuador1, mote2.configs.at4)==0) mote2.configs.est4=mote2.regras[k].estado1;
                    }
                    if(strlen(mote2.regras[k].atuador2)!=0) 
                    {   
                        if(strcmp(mote2.regras[k].atuador2, mote2.configs.at1)==0) mote2.configs.est1=mote2.regras[k].estado2;
                        if(strcmp(mote2.regras[k].atuador2, mote2.configs.at2)==0) mote2.configs.est2=mote2.regras[k].estado2;
                        if(strcmp(mote2.regras[k].atuador2, mote2.configs.at3)==0) mote2.configs.est3=mote2.regras[k].estado2;
                        if(strcmp(mote2.regras[k].atuador2, mote2.configs.at4)==0) mote2.configs.est4=mote2.regras[k].estado2;
                    }
                    if(strlen(mote2.regras[k].atuador3)!=0)
                    {
                        if(strcmp(mote2.regras[k].atuador3, mote2.configs.at1)==0) mote2.configs.est1=mote2.regras[k].estado3;
                        if(strcmp(mote2.regras[k].atuador3, mote2.configs.at2)==0) mote2.configs.est2=mote2.regras[k].estado3;
                        if(strcmp(mote2.regras[k].atuador3, mote2.configs.at3)==0) mote2.configs.est3=mote2.regras[k].estado3;
                        if(strcmp(mote2.regras[k].atuador3, mote2.configs.at4)==0) mote2.configs.est4=mote2.regras[k].estado3;
                    }
                    if(strlen(mote2.regras[k].atuador4)!=0)
                    {
                        if(strcmp(mote2.regras[k].atuador4, mote2.configs.at1)==0) mote2.configs.est1=mote2.regras[k].estado4;
                        if(strcmp(mote2.regras[k].atuador4, mote2.configs.at2)==0) mote2.configs.est2=mote2.regras[k].estado4;
                        if(strcmp(mote2.regras[k].atuador4, mote2.configs.at3)==0) mote2.configs.est3=mote2.regras[k].estado4;
                        if(strcmp(mote2.regras[k].atuador4, mote2.configs.at4)==0) mote2.configs.est4=mote2.regras[k].estado4;
                    }
                        
                    if(strcmp(mote2.regras[k].sensor1, "Tem2")==0) linearidade(4);
                    if(strcmp(mote2.regras[k].sensor1, "Hum2")==0) linearidade(5);
                    if(strcmp(mote2.regras[k].sensor1, "Cor2")==0) linearidade(3);

                    if(strcmp(mote2.regras[k].sensor2, "Tem2")==0) linearidade(4);
                    if(strcmp(mote2.regras[k].sensor2, "Hum2")==0) linearidade(5);
                    if(strcmp(mote2.regras[k].sensor2, "Cor2")==0) linearidade(3);
                    if(strcmp(mote2.regras[k].sensor3, "Tem2")==0) linearidade(4);
                    if(strcmp(mote2.regras[k].sensor3, "Hum2")==0) linearidade(5);
                    if(strcmp(mote2.regras[k].sensor3, "Cor2")==0) linearidade(3);
                    if(strcmp(mote2.regras[k].sensor4, "Tem2")==0) linearidade(4);
                    if(strcmp(mote2.regras[k].sensor4, "Hum2")==0) linearidade(5);
                    if(strcmp(mote2.regras[k].sensor4, "Cor2")==0) linearidade(3);
                }

                
              
                auxiliar1[k]=mote2.regras[k].ativa;
            }
          
            
        }
        
        if(hex_to_decimal(mote_id)==3)
        {
            for(k=0;k<8;++k)
            {
                
                calc_logico=verifica_regras(&mote3.regras[k], 1, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current);
                
                if((strcmp(mote3.regras[k].logico1, "\0")!=0) && calc_logico==1)
                {
                    if (strcmp(mote3.regras[k].logico1, "AND")==0)
                    {
                        calc_logico= (calc_logico && verifica_regras(&mote3.regras[k], 2, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                        
                        if((strcmp(mote3.regras[k].logico2, "\0")!=0) && calc_logico==1)
                        {
                            if (strcmp(mote3.regras[k].logico2, "AND")==0)
                            {
                                calc_logico= (calc_logico && verifica_regras(&mote3.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));

                                if((strcmp(mote3.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote3.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote3.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote3.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    
                                }
                            }
                            else
                            {
                                calc_logico= (calc_logico || verifica_regras(&mote3.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                if((strcmp(mote3.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote3.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote3.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote3.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        calc_logico= (calc_logico || verifica_regras(&mote3.regras[k], 2, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                        if((strcmp(mote3.regras[k].logico2, "\0")!=0) && calc_logico==1)
                        {
                            if (strcmp(mote3.regras[k].logico2, "AND")==0)
                            {
                                calc_logico= (calc_logico && verifica_regras(&mote3.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                if((strcmp(mote3.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote3.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote3.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote3.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                }

                            }
                            else
                            {
                                calc_logico= (calc_logico || verifica_regras(&mote3.regras[k], 3, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                if((strcmp(mote3.regras[k].logico3, "\0")!=0) && calc_logico==1)
                                {
                                    if (strcmp(mote3.regras[k].logico3, "AND")==0)
                                    {
                                        calc_logico= (calc_logico && verifica_regras(&mote3.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                    else
                                    {
                                        calc_logico= (calc_logico || verifica_regras(&mote3.regras[k], 4, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current));
                                    }
                                }
                            }
                        }

                    }
    
                }

                if (calc_logico==1)
                {   
                    mote3.regras[k].ativa=1;
                }
                if(calc_logico == 0)
                {
                    mote3.regras[k].ativa=0;
                }
              
              
                if((mote3.regras[k].ativa==1 && auxiliar2[k]==0) )
                {
                    
                    if(strlen(mote3.regras[k].atuador1)!=0) 
                    {
                        if(strcmp(mote3.regras[k].atuador1, mote3.configs.at1)==0) mote3.configs.est1=mote3.regras[k].estado1;
                        if(strcmp(mote3.regras[k].atuador1, mote3.configs.at2)==0) mote3.configs.est2=mote3.regras[k].estado1;
                        if(strcmp(mote3.regras[k].atuador1, mote3.configs.at3)==0) mote3.configs.est3=mote3.regras[k].estado1;
                        if(strcmp(mote3.regras[k].atuador1, mote3.configs.at4)==0) mote3.configs.est4=mote3.regras[k].estado1;
                    }
                    if(strlen(mote3.regras[k].atuador2)!=0) 
                    {
                        if(strcmp(mote3.regras[k].atuador2, mote3.configs.at1)==0) mote3.configs.est1=mote3.regras[k].estado2;
                        if(strcmp(mote3.regras[k].atuador2, mote3.configs.at2)==0) mote3.configs.est2=mote3.regras[k].estado2;
                        if(strcmp(mote3.regras[k].atuador2, mote3.configs.at3)==0) mote3.configs.est3=mote3.regras[k].estado2;
                        if(strcmp(mote3.regras[k].atuador2, mote3.configs.at4)==0) mote3.configs.est4=mote3.regras[k].estado2;
                    }
                    if(strlen(mote3.regras[k].atuador3)!=0)
                    {
                        if(strcmp(mote3.regras[k].atuador3, mote3.configs.at1)==0) mote3.configs.est1=mote3.regras[k].estado3;
                        if(strcmp(mote3.regras[k].atuador3, mote3.configs.at2)==0) mote3.configs.est2=mote3.regras[k].estado3;
                        if(strcmp(mote3.regras[k].atuador3, mote3.configs.at3)==0) mote3.configs.est3=mote3.regras[k].estado3;
                        if(strcmp(mote3.regras[k].atuador3, mote3.configs.at4)==0) mote3.configs.est4=mote3.regras[k].estado3;
                    }
                    if(strlen(mote3.regras[k].atuador4)!=0)
                    {
                        if(strcmp(mote3.regras[k].atuador4, mote3.configs.at1)==0) mote3.configs.est1=mote3.regras[k].estado4;
                        if(strcmp(mote3.regras[k].atuador4, mote3.configs.at2)==0) mote3.configs.est2=mote3.regras[k].estado4;
                        if(strcmp(mote3.regras[k].atuador4, mote3.configs.at3)==0) mote3.configs.est3=mote3.regras[k].estado4;
                        if(strcmp(mote3.regras[k].atuador4, mote3.configs.at4)==0) mote3.configs.est4=mote3.regras[k].estado4;
                    }

                }

                
              
                auxiliar2[k]=mote3.regras[k].ativa;
                
            }
          
            
        }
        
        
        
        
        
        
        Print_Matrix(mote1.configs.est1, mote1.configs.est2, mote1.configs.est3, mote1.configs.est4, mote2.configs.est1, mote2.configs.est2, mote3.configs.est1, mote3.configs.est2);

        fclose(fp);
       
        memset(mote_id, '\0', sizeof mote_id);
        memset(raw_voltage, '\0', sizeof raw_voltage);
        memset(raw_visible_light, '\0', sizeof raw_visible_light);
        memset(raw_current, '\0', sizeof raw_current);
        memset(raw_temperature, '\0', sizeof raw_temperature);
        memset(raw_humidity, '\0', sizeof raw_humidity);
    }
    
    
   
}



int hex_to_decimal(char byte[])
{
    int i, j, dec=0, base=1;
    for (i=3; i>=0; i--)
    {
            if(byte[i]>='0' && byte[i]<='9')
            {
                dec += (byte[i] - 48)*base;
                base = base*16;
            }
            else if(byte[i]>='A' && byte[i]<='F')
            {
                dec += (byte[i] - 55)*base;
                base=base*16;
            }   
        
    }
    return dec;

}

float voltage (char mensagem[])
{
    return hex_to_decimal(mensagem)*1.5*2/4096;
}

float temperature (char mensagem[])
{
    return -39.6 +0.01*hex_to_decimal(mensagem);
}

float relative_humidity(char mensagem[])
{
    return (-2.0468+0.0367*hex_to_decimal(mensagem) + -1.5955*(0.000001 )* ((hex_to_decimal(mensagem))*hex_to_decimal(mensagem)));
}

float humidity_compensated_temperature(char hum[], char temp[], float lin)
{
    return (hex_to_decimal(temp)-25)*(0.01+0.00008*hex_to_decimal(hum))+ lin;
}

float visible_light (char mensagem[])
{   
    return 0.625 * 10000 * 1.5 * hex_to_decimal(mensagem)/(4096);
}

float current (char mensagem[])
{
    return 0.769 *1000 * 1.5 * hex_to_decimal(mensagem)/4096;
}

void imprimir (char *mensagem, FILE * file)
{
    fprintf(file, "%s", mensagem);
}

void guarda_sensor(struct regra *reg, char *aux, int ident)
{
    if (ident == 1)
    {
        strncpy(reg->sensor1, aux, 4);
        reg->sensor1[4]='\0';
        
        strncpy(reg->comp1, aux+4, 1);
        reg->comp1[1] = '\0';
    
        reg->valor1 = atoi(aux+5);
    }
    if (ident == 2)
    {
        strncpy(reg->sensor2, aux, 4);
        reg->sensor2[4]='\0';
        
        strncpy(reg->comp2, aux+4, 1);
        reg->comp2[1] = '\0';
    
        reg->valor2 = atoi(aux+5);
    }
    if (ident == 3)
    {
        strncpy(reg->sensor3, aux, 4);
        reg->sensor3[4]='\0';
        
        strncpy(reg->comp3, aux+4, 1);
        reg->comp3[1] = '\0';
    
        reg->valor3 = atoi(aux+5);
    }

    if (ident == 4)
    {
        strncpy(reg->sensor4, aux, 4);
        reg->sensor4[4]='\0';
        
        strncpy(reg->comp4, aux+4, 1);
        reg->comp4[1] = '\0';
    
        reg->valor4 = atoi(aux+5);
    }

}

void guarda_atuadores(struct regra *reg, char *aux, int ident)
{
    if(ident==1)
    {
        strncpy(reg->atuador1, aux, 4);
        reg->atuador1[4]='\0';

        if(strcmp(aux+5, "OFF")==0)
            reg->estado1 = 0;
        if(strcmp(aux+5, "ON")==0)
            reg->estado1=1;
    }
    if(ident==2)
    {
        strncpy(reg->atuador2, aux, 4);
        reg->atuador2[4]='\0';

        if(strcmp(aux+5, "OFF")==0)
            reg->estado2 = 0;
        if(strcmp(aux+5, "ON")==0)
            reg->estado2=1;
        
    }
    if(ident==3)
    {
        strncpy(reg->atuador3, aux, 4);
        reg->atuador3[4]='\0';

        if(strcmp(aux+5, "OFF")==0)
            reg->estado3 = 0;
        if(strcmp(aux+5, "ON")==0)
            reg->estado3=1;
    }
    if(ident==4)
    {
        strncpy(reg->atuador4, aux, 4);
        reg->atuador4[4]='\0';

        if(strcmp(aux+5, "OFF")==0)
            reg->estado4 = 0;
        if(strcmp(aux+5, "ON")==0)
            reg->estado4=1;
    }
}

void mote_init(struct mote *mot)
{
    
    int i;
    memset(mot->configs.nome, '\0', sizeof mot->configs.nome);
    memset(mot->configs.sensores, '\0', sizeof mot->configs.sensores);
    memset(mot->configs.at1, '\0', sizeof mot->configs.at1);
    memset(mot->configs.at2, '\0', sizeof mot->configs.at2);
    memset(mot->configs.at3, '\0', sizeof mot->configs.at3);
    memset(mot->configs.at4, '\0', sizeof mot->configs.at4);
    mot->configs.est1 = 0;
    mot->configs.est2 = 0;
    mot->configs.est3 = 0;
    mot->configs.est4 = 0;
   


    for(i=0; i<8; i++)
    {
        
        memset(mot->regras[i].sensor1, '\0', sizeof mot->regras[i].sensor1);
        memset(mot->regras[i].sensor2, '\0', sizeof mot->regras[i].sensor2);
        memset(mot->regras[i].sensor3, '\0', sizeof mot->regras[i].sensor3);
        memset(mot->regras[i].sensor4, '\0', sizeof mot->regras[i].sensor4);

        memset(mot->regras[i].comp1, '\0', sizeof mot->regras[i].comp1);
        memset(mot->regras[i].comp2, '\0', sizeof mot->regras[i].comp2);
        memset(mot->regras[i].comp3, '\0', sizeof mot->regras[i].comp3);
        memset(mot->regras[i].comp4, '\0', sizeof mot->regras[i].comp4);

        memset(mot->regras[i].logico1, '\0', sizeof mot->regras[i].logico1);
        memset(mot->regras[i].logico2, '\0', sizeof mot->regras[i].logico2);
        memset(mot->regras[i].logico3, '\0', sizeof mot->regras[i].logico3);


        memset(mot->regras[i].atuador1, '\0', sizeof mot->regras[i].atuador1);
        memset(mot->regras[i].atuador2, '\0', sizeof mot->regras[i].atuador2);
        memset(mot->regras[i].atuador3, '\0', sizeof mot->regras[i].atuador3);
        memset(mot->regras[i].atuador4, '\0', sizeof mot->regras[i].atuador4);
        mot->regras[i].valor1 = 0;
        mot->regras[i].valor2 = 0;
        mot->regras[i].valor3 = 0;
        mot->regras[i].valor4 = 0;
        mot->regras[i].estado1 = 0;
        mot->regras[i].estado2 = 0;
        mot->regras[i].estado3 = 0;
        mot->regras[i].estado4 = 0;
        mot->regras[i].ativa = 0;
        

        
    }
    
}


int verifica_regras(struct regra *reg, int num, char *raw_temperature, char *raw_humidity, char *raw_visible_light, char *raw_voltage, char *raw_current)
{   
    if(num == 1)
    {
        
        if((strcmp(reg->sensor1, "Tem1")==0) || (strcmp(reg->sensor1, "Tem2")==0))
        {
            if (strcmp(reg->comp1, ">")==0)
            
            return (temperature(raw_temperature)>(reg->valor1));
            
            else return (temperature(raw_temperature)<reg->valor1);
        }
        
        if((strcmp(reg->sensor1, "Hum1")==0) || (strcmp(reg->sensor1, "Hum2")==0))
        {
            if (strcmp(reg->comp1, ">")==0)
            return (relative_humidity(raw_humidity)>reg->valor1);
            else return (relative_humidity(raw_humidity)<reg->valor1);
        }

        if((strcmp(reg->sensor1, "Lum1")==0) || (strcmp(reg->sensor1, "Lum2")==0) || (strcmp(reg->sensor1, "Lum3")==0))
        {   
            if (strcmp(reg->comp1, ">")==0)
            return (visible_light(raw_visible_light)>reg->valor1);
            else return (visible_light(raw_visible_light)<reg->valor1);
        }

        if((strcmp(reg->sensor1, "Ten1")==0) || (strcmp(reg->sensor1, "Ten3")==0))
        {
            if (strcmp(reg->comp1, ">")==0)
            return (voltage(raw_voltage)>reg->valor1);
            else return (voltage(raw_voltage)<reg->valor1);
        }

        if(strcmp(reg->sensor1, "Cor2")==0)
        {
            if (strcmp(reg->comp1, ">")==0)
            return (current(raw_current)>reg->valor1);
            else return (current(raw_current)<reg->valor1);
        }
    }

    if(num == 2)
    {
        if((strcmp(reg->sensor2, "Tem1")==0) || (strcmp(reg->sensor2, "Tem2")==0))
        {
            if (strcmp(reg->comp2, ">")==0)
            return (temperature(raw_temperature)>reg->valor2);
            else return (temperature(raw_temperature)<reg->valor2);
        }
        
        if((strcmp(reg->sensor2, "Hum1")==0) || (strcmp(reg->sensor2, "Hum2")==0))
        {
            if (strcmp(reg->comp2, ">")==0)
            return (relative_humidity(raw_humidity)>reg->valor2);
            else return (relative_humidity(raw_humidity)<reg->valor2);
        }

        if((strcmp(reg->sensor2, "Lum1")==0) || (strcmp(reg->sensor2, "Lum2")==0) || (strcmp(reg->sensor2, "Lum3")==0))
        {
            if (strcmp(reg->comp2, ">")==0)
            return (visible_light(raw_visible_light)>reg->valor2);
            else return (visible_light(raw_visible_light)<reg->valor2);
        }

        if((strcmp(reg->sensor2, "Ten1")==0) || (strcmp(reg->sensor2, "Ten3")==0))
        {
            if (strcmp(reg->comp2, ">")==0)
            return (voltage(raw_voltage)>reg->valor2);
            else return (voltage(raw_voltage)<reg->valor2);
        }

        if(strcmp(reg->sensor2, "Cor2")==0)
        {
            if (strcmp(reg->comp2, ">")==0)
            return (current(raw_current)>reg->valor2);
            else return (current(raw_current)<reg->valor2);
        }
    }

        if(num == 3)
    {
        if((strcmp(reg->sensor3, "Tem1")==0) || (strcmp(reg->sensor3, "Tem2")==0))
        {
            if (strcmp(reg->comp3, ">")==0)
            return (temperature(raw_temperature)>reg->valor3);
            else return (temperature(raw_temperature)<reg->valor3);
        }
        
        if((strcmp(reg->sensor3, "Hum1")==0) || (strcmp(reg->sensor3, "Hum2")==0))
        {
            if (strcmp(reg->comp3, ">")==0)
            return (relative_humidity(raw_humidity)>reg->valor3);
            else return (relative_humidity(raw_humidity)<reg->valor3);
        }

        if((strcmp(reg->sensor3, "Lum1")==0) || (strcmp(reg->sensor3, "Lum2")==0) || (strcmp(reg->sensor3, "Lum3")==0))
        {
            if (strcmp(reg->comp3, ">")==0)
            return (visible_light(raw_visible_light)>reg->valor3);
            else return (visible_light(raw_visible_light)<reg->valor3);
        }

        if((strcmp(reg->sensor3, "Ten1")==0) || (strcmp(reg->sensor3, "Ten3")==0))
        {
            if (strcmp(reg->comp3, ">")==0)
            return (voltage(raw_voltage)>reg->valor3);
            else return (voltage(raw_voltage)<reg->valor3);
        }

        if(strcmp(reg->sensor3, "Cor2")==0)
        {
            if (strcmp(reg->comp3, ">")==0)
            return (current(raw_current)>reg->valor3);
            else return (current(raw_current)<reg->valor3);
        }
    }

    if(num == 4)
    {
        if((strcmp(reg->sensor4, "Tem1")==0) || (strcmp(reg->sensor4, "Tem2")==0))
        {
            if (strcmp(reg->comp4, ">")==0)
            return (temperature(raw_temperature)>reg->valor4);
            else return (temperature(raw_temperature)<reg->valor4);
        }
        
        if((strcmp(reg->sensor4, "Hum1")==0) || (strcmp(reg->sensor4, "Hum2")==0))
        {
            if (strcmp(reg->comp4, ">")==0)
            return (relative_humidity(raw_humidity)>reg->valor4);
            else return (relative_humidity(raw_humidity)<reg->valor4);
        }

        if((strcmp(reg->sensor4, "Lum1")==0) || (strcmp(reg->sensor4, "Lum2")==0) || (strcmp(reg->sensor4, "Lum3")==0))
        {
            if (strcmp(reg->comp4, ">")==0)
            return (visible_light(raw_visible_light)>reg->valor4);
            else return (visible_light(raw_visible_light)<reg->valor4);
        }

        if((strcmp(reg->sensor4, "Ten1")==0) || (strcmp(reg->sensor4, "Ten3")==0))
        {
            if (strcmp(reg->comp4, ">")==0)
            return (voltage(raw_voltage)>reg->valor4);
            else return (voltage(raw_voltage)<reg->valor4);
        }

        if(strcmp(reg->sensor4, "Cor2")==0)
        {
            if (strcmp(reg->comp4, ">")==0)
            return (current(raw_current)>reg->valor4);
            else return (current(raw_current)<reg->valor4);
        }
    }
    return 0;
}


void Print_Matrix(int LED_FAC, int A_C, int DEHUMI, int CONV_BELT, int LED_OVEN, int OVEN, int A_C_3, int DEHUMI_3){
    FILE *rgb_file;

    rgb_file =fopen("/tmp/ttyV12", "w");
    if(rgb_file == NULL)
    {
        printf("ERRO1\n");
    }

    
int i, j,k;

//1st
    fprintf(rgb_file, "%s", "["BLUE",");
    for(i=0;i<19;i++){
        fprintf(rgb_file, "%s", BLUE",");
    }

//adicional
for(j=0;j<2;j++){
    fprintf(rgb_file, "%s", BLUE",");
    fprintf(rgb_file, "%s", BLUE",");
        if(A_C_3==-1){
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", BLUE",");
            }
        }
        else if(A_C_3){
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", GREEN",");
            }
        }
        else{
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", RED",");
            }
        }
        if(DEHUMI_3==-1){
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", BLUE",");
            }
        }
        else if(DEHUMI_3){
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", GREEN",");
            }
        }
        else{
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", RED",");
            }
        }
    for(i=0;i<14;i++){
        fprintf(rgb_file, "%s", BLUE",");
    }
}
    

//1
for(j=0;j<2;j++){
    for(i=0;i<20;i++){
        fprintf(rgb_file, "%s", BLUE",");
    }
}
//2
    for(j=0;j<2;j++){
        for(i=0;i<11;i++){
            fprintf(rgb_file, "%s", BLUE",");
        }
        for(i=0;i<8;i++){
            fprintf(rgb_file, "%s", BLACK",");
        }
        fprintf(rgb_file, "%s", BLUE",");
    }
//3
for(k=0;k<2;k++){
    fprintf(rgb_file, "%s", BLUE",");
    fprintf(rgb_file, "%s", BLUE",");
    for(j=0;j<3;j++){    
        for(i=0;i<2;i++){
            fprintf(rgb_file, "%s", GRAY",");
        }
        fprintf(rgb_file, "%s", BLUE",");
    }
    fprintf(rgb_file, "%s", BLACK",");
    for(i=0;i<6;i++){
        fprintf(rgb_file, "%s", GRAY",");
    }
    fprintf(rgb_file, "%s", BLACK",");
    fprintf(rgb_file, "%s", BLUE",");
}
//4
for(k=0;k<2;k++){
    fprintf(rgb_file, "%s", BLUE",");
    fprintf(rgb_file, "%s", BLUE",");
    for(j=0;j<3;j++){    
        for(i=0;i<2;i++){
            fprintf(rgb_file, "%s", GRAY",");
        }
        fprintf(rgb_file, "%s", BLUE",");
    }
    fprintf(rgb_file, "%s", BLACK",");
    fprintf(rgb_file, "%s", GRAY",");
    if(LED_OVEN){
        for(i=0;i<2;i++){
            fprintf(rgb_file, "%s", GREEN",");
        }
    }
    else{
        for(i=0;i<2;i++){
            fprintf(rgb_file, "%s", RED",");
        }
    }
    if(OVEN){
        for(i=0;i<2;i++){
            fprintf(rgb_file, "%s", GREEN",");
        }
    }
    else{
        for(i=0;i<2;i++){
            fprintf(rgb_file, "%s", RED",");
        }
    }
    fprintf(rgb_file, "%s", GRAY",");
    fprintf(rgb_file, "%s", BLACK",");
    fprintf(rgb_file, "%s", BLUE",");
} 


    
//3
    for(k=0;k<2;k++){
    fprintf(rgb_file, "%s", BLUE",");
    fprintf(rgb_file, "%s", BLUE",");
    for(j=0;j<3;j++){    
        for(i=0;i<2;i++){
            fprintf(rgb_file, "%s", GRAY",");
        }
        fprintf(rgb_file, "%s", BLUE",");
    }
    fprintf(rgb_file, "%s", BLACK",");
    for(i=0;i<6;i++){
        fprintf(rgb_file, "%s", GRAY",");
    }
    fprintf(rgb_file, "%s", BLACK",");
    fprintf(rgb_file, "%s", BLUE",");
}
   
//2
    for(j=0;j<2;j++){
        for(i=0;i<11;i++){
            fprintf(rgb_file, "%s", BLUE",");
        }
        for(i=0;i<8;i++){
            fprintf(rgb_file, "%s", BLACK",");
        }
        fprintf(rgb_file, "%s", BLUE",");
    }


//1
    for(i=0;i<20;i++){
        fprintf(rgb_file, "%s", BLUE",");
    }

//5
    for(j=0;j<2;j++){
        fprintf(rgb_file, "%s", BLUE",");
        fprintf(rgb_file, "%s", BLUE",");
        if(LED_FAC){
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", GREEN",");
            }
        }
        else{
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", RED",");
            }
        }

        if(A_C){
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", GREEN",");
            }
        }
        else{
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", RED",");
            }
        }

        if(DEHUMI){
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", GREEN",");
            }
        }
        else{
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", RED",");
            }
        }
        if(CONV_BELT){
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", GREEN",");
            }
        }
        else{
            for(i=0;i<2;i++){
                fprintf(rgb_file, "%s", RED",");
            }
        }
        for(i=0;i<10;i++){
            fprintf(rgb_file, "%s", BLUE",");
        }
    }

    for(i=0;i<20;i++){
        fprintf(rgb_file, "%s", BLUE",");
    }

 //END  
    for(i=0;i<19;i++){
        fprintf(rgb_file, "%s", BLUE",");
    }
    fprintf(rgb_file, "%s", BLUE"]\n");
    fclose(rgb_file);
}

void linearidade(int caso)    
{
    FILE *msg2_L, *msg1_L;
    char a;
        
    if((caso==3) || (caso==4) || (caso==5)){
        
        msg2_L=fopen("./msgcreator2/MsgCreatorConf.txt", "r+");
        if(msg2_L==NULL)    printf("Didn't open file");
        
        if(caso==3){
            
            fseek(msg2_L, 75, SEEK_SET);
            fscanf(msg2_L, "%c", &a);
          
           
            if(a==43){
               
                fseek(msg2_L, 75, SEEK_SET);
                fprintf(msg2_L, "-");
            }
            if(a==45){
                fseek(msg2_L, 75, SEEK_SET);
                fprintf(msg2_L, "+");
            }       
        }
        if(caso==4){
            
            fseek(msg2_L, 97, SEEK_SET);
            fscanf(msg2_L, "%c", &a);
            
            if(a==43){
                fseek(msg2_L, 97, SEEK_SET);
                fprintf(msg2_L, "-");
            }
            if(a==45){
                fseek(msg2_L, 97, SEEK_SET);
                fprintf(msg2_L, "+");
            }    
        }
        if(caso==5){
            
            fseek(msg2_L, 117, SEEK_SET);
            fscanf(msg2_L, "%c", &a);
            
            if(a==43){
                fseek(msg2_L, 117, SEEK_SET);
                fprintf(msg2_L, "-");
            }
            if(a==45){
                fseek(msg2_L, 117, SEEK_SET);
                fprintf(msg2_L, "+");
            }    
        }
        fclose(msg2_L);
    }
    if((caso==1) || (caso==2)){
        
        msg1_L=fopen("MsgCreatorConf.txt", "r+");
        if(msg1_L==NULL)    printf("Didn't open file");
        
        if(caso==1){
            
            fseek(msg1_L, 92, SEEK_SET);
            fscanf(msg1_L, "%c", &a);
            
            if(a==43){
                fseek(msg1_L, 92, SEEK_SET);
                fprintf(msg1_L, "-");
            }    
            if(a==45){
                fseek(msg1_L, 92, SEEK_SET);
                fprintf(msg1_L, "+");
                
            }    
        }
        if(caso==2){
            fseek(msg1_L, 111, SEEK_SET);
            fscanf(msg1_L, "%c", &a);
            
            if(a==43){
                fseek(msg1_L, 111, SEEK_SET);
                fprintf(msg1_L, "-");
                
            }
            if(a==45){
                fseek(msg1_L, 111, SEEK_SET);
                fprintf(msg1_L, "+");
                
            }  
        }
        fclose(msg1_L);
    }
}

