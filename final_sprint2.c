#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <postgresql/libpq-fe.h>


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
    char sens1[5];
    char sens2[5];
    char sens3[5];
    char sens4[5];

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
void escreve_sensor(int y,  PGresult *res, PGconn *conn, struct mote *mot);
void escreve_atuadores(int y, PGresult *res, PGconn *conn, struct mote *mot);
void escreve_input(int y, PGresult *res, PGconn *conn, struct mote *mot,int num);
void escreve_sensoresinput(int y, PGresult *res, PGconn *conn, char *sensor, char *num, struct mote *mot);
void escreve_regra(int y,  PGresult *res, PGconn *conn, struct mote *mot, int num);
void escreve_logico(int y, PGresult *res, PGconn *conn, char *logico, int regra, int num);
void escreve_output(int y, PGresult *res, PGconn *conn, struct mote *mot,int num);
void escreve_regraoutput(char *output, int num, PGresult *res, PGconn *conn);
void escreve_outputatuador( PGresult *res, PGconn *conn, char *sensor, char *num);
void escreve_valor(int y, struct mote *mot, PGresult *res, PGconn *conn,  char *temp, char *hum, char *luz, char *volt, char *curr, int *cont);
void escreve_sensoresvalor(PGresult *res, PGconn *conn, char *sensor, char *num);
void escreve_estado(int y, PGresult *res, PGconn *conn, struct mote *mot, int estado, char *atua, int *num);
void escreve_atuadorestado(PGresult *res, PGconn *conn, char* teste, char* atuador);

int main()
{
    PGconn *conn; 
	PGresult *res, *res1;
	const char *dbconn;
    FILE *fp, *sensor_rules, *sensor_configs;
    int i=0, j=0, k, y, cont=0, cont1=0, cont2=0, cursor , calc_logico=0, old_estado1=0, atua1=0, atua2=0, atua3=0, atua4=0, atua5=0, auxiliar[7]={0,0,0,0,0,0}, auxiliar1[7]={0,0,0,0,0,0}, auxiliar2[7]={0,0,0,0,0,0}, cont_valor=1, cont_estado=1;
    char str[150], *bytes, mote_id[5], raw_voltage[5], raw_visible_light[5], raw_current[5], raw_temperature[5], raw_humidity[5], nome[150], sensores[150], atuadores[200], *aux_sensor, *aux_atuador, at_aux[5], *test[24], sens_aux[150];
    struct mote mote1;
    struct mote mote2;
    struct mote mote3; //MAXIMO 3 MOTES

    //SELECT "comparador", count(*) AS "count" FROM "inputt" GROUP BY "comparador" ORDER BY "comparador"
	dbconn = "host = 'db.fe.up.pt' dbname = 'sinf2021a31' user = 'sinf2021a31' password = 'vDkRsnVU'";
	//EXAMPLE : dbconn = "host = 'db.fe.up.pt' dbname = 'sinf1920e32' user = 'sinf1920e32' password = 'QWTTIjZl'";

	conn = PQconnectdb(dbconn);
	PQexec(conn,"SET search_path TO db_sinfa31");
	
	if (!conn){
		fprintf(stderr, "libpq error: PQconnectdb returned NULL. \n\n");
		PQfinish(conn);
		exit(1);
	}
	
	else if (PQstatus(conn) != CONNECTION_OK){
		fprintf(stderr, "Connection to DB failed: %s", PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);
	}
    else {
        printf("Connection OK \n");
    
    mote_init(&mote1);
    mote_init(&mote2);
    mote2.configs.est2=1;
    mote_init(&mote3);
    mote3.configs.est1=-1;
    mote3.configs.est2=-1;

    

    sensor_configs = fopen("SensorConfigurations.txt", "r");

    if (sensor_configs == NULL)
    {
        printf("ERRO\n");
        return (-1);
    }
    
    while (1)
    {
   
        if( fscanf(sensor_configs, "%[^:]:%[^:]:%s\n", mote1.configs.nome, sens_aux, at_aux) !=EOF)
        {
        strcpy(mote1.configs.sens1,strtok(sens_aux, ","));
        strcpy(mote1.configs.sens2, strtok(NULL, ","));
        strcpy(mote1.configs.sens3, strtok(NULL, ","));
        strcpy(mote1.configs.sens4, strtok(NULL, ":"));

       

        strcpy(mote1.configs.at1,strtok(at_aux, ","));
        strcpy(mote1.configs.at2, strtok(NULL, ","));
        strcpy(mote1.configs.at3, strtok(NULL, ","));
        strcpy(mote1.configs.at4, strtok(NULL, "\0"));

        }

        if(fscanf(sensor_configs, "%[^:]:%[^:]:%s\n", mote2.configs.nome, sens_aux, at_aux)!= EOF)
        {
        strcpy(mote2.configs.at1,strtok(at_aux, ","));
        strcpy(mote2.configs.at2, strtok(NULL, "\0"));

        strcpy(mote2.configs.sens1,strtok(sens_aux, ","));
        strcpy(mote2.configs.sens2, strtok(NULL, ","));
        strcpy(mote2.configs.sens3, strtok(NULL, ","));
        strcpy(mote2.configs.sens4, strtok(NULL, "\0"));
        }
        if(fscanf(sensor_configs, "%[^:]:%[^:]:%s\n", mote3.configs.nome, sens_aux, at_aux)!= EOF)
        {
        strcpy(mote3.configs.at1,strtok(at_aux, ","));
        strcpy(mote3.configs.at2, strtok(NULL, "\0"));

        strcpy(mote3.configs.sens1,strtok(sens_aux, ","));
        strcpy(mote3.configs.sens2, strtok(NULL, "\0"));
        }

        break;
    }
    fclose(sensor_configs);
    
    escreve_sensor(1, res, conn, &mote1);
    escreve_sensor(2, res, conn, &mote2);
    escreve_sensor(3, res, conn, &mote3);

    escreve_atuadores(1, res, conn, &mote1);
    escreve_atuadores(2, res, conn, &mote2);
    escreve_atuadores(3, res, conn, &mote3);



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
   
    escreve_regra(1, res, conn, &mote1, 1);
    escreve_regra(2, res, conn, &mote2, 7);
    escreve_regra(3, res, conn, &mote3, 13);

   
    escreve_input(1, res, conn, &mote1, 1);
    escreve_input(2, res, conn, &mote2, 9);
    escreve_input(3, res, conn, &mote3, 17);

    escreve_output(1, res, conn, &mote1, 1);
    escreve_output(2, res, conn, &mote2, 9);
    escreve_output(3, res, conn, &mote3, 17);

    printf("AQUI\n");

    while(1)
    {
        fp =fopen("/tmp/ttyV11", "r+");
        if (fp == NULL) 
        {
            printf("ERRO MSG\n");
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

    
        printf("OLA %d\n", hex_to_decimal(mote_id));
        
        
        

       //MOTE_ID

        if(hex_to_decimal(mote_id)==1)
        {
            printf("OLA\n");
            escreve_valor(1, &mote1, res, conn,raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current, &cont_valor);
            printf("PASSOU\n");
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
            escreve_valor(2, &mote2, res, conn,raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current, &cont_valor);
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
            escreve_valor(3, &mote3, res, conn,raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current, &cont_valor);
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
        
        
        escreve_estado(1, res, conn, &mote1, mote1.configs.est1, mote1.configs.at1, &cont_estado);
        escreve_estado(1, res, conn, &mote1, mote1.configs.est2, mote1.configs.at2, &cont_estado);
        escreve_estado(1, res, conn, &mote1, mote1.configs.est3, mote1.configs.at3, &cont_estado);
        escreve_estado(1, res, conn, &mote1, mote1.configs.est4, mote1.configs.at4, &cont_estado);
        escreve_estado(2, res, conn, &mote2, mote1.configs.est1, mote2.configs.at1, &cont_estado);
        escreve_estado(2, res, conn, &mote2, mote1.configs.est2, mote2.configs.at2, &cont_estado);
        escreve_estado(3, res, conn, &mote3, mote1.configs.est1, mote3.configs.at1, &cont_estado);
        escreve_estado(3, res, conn, &mote3, mote1.configs.est2, mote3.configs.at2, &cont_estado);


        
        
        Print_Matrix(mote1.configs.est1, mote1.configs.est2, mote1.configs.est3, mote1.configs.est4, mote2.configs.est1, mote2.configs.est2, mote3.configs.est1, mote3.configs.est2);

		
        fclose(fp);
       
        memset(mote_id, '\0', sizeof mote_id);
        memset(raw_voltage, '\0', sizeof raw_voltage);
        memset(raw_visible_light, '\0', sizeof raw_visible_light);
        memset(raw_current, '\0', sizeof raw_current);
        memset(raw_temperature, '\0', sizeof raw_temperature);
        memset(raw_humidity, '\0', sizeof raw_humidity);

    }
    PQfinish(conn);
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
    memset(mot->configs.sens1, '\0', sizeof mot->configs.sens1);
    memset(mot->configs.sens2, '\0', sizeof mot->configs.sens2);
    memset(mot->configs.sens3, '\0', sizeof mot->configs.sens3);
    memset(mot->configs.sens4, '\0', sizeof mot->configs.sens4);
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
            fscanf(msg2_L, "%c", &a);printf("METEU4\n");
            
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
void escreve_sensor(int y,  PGresult *res, PGconn *conn, struct mote *mot)
{

    char mote_conta[150], *values[150];
    sprintf(mote_conta, "%d", y);
   
    if(y==1 || y==2)
    {
        if(y==1) res=PQexec(conn, "INSERT INTO mote (moteid) VALUES(1)");
        
        if(y==2) res=PQexec(conn, "INSERT INTO mote (moteid) VALUES(2)");

        values[0]= mot->configs.sens1;
        values[1]= mote_conta;

        res = PQexecParams(conn, "INSERT INTO sensor (sensor_nome, moteid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);

        values[0]= mot->configs.sens2;
        res = PQexecParams(conn, "INSERT INTO sensor (sensor_nome, moteid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);

        values[0]= mot->configs.sens3;
        res = PQexecParams(conn, "INSERT INTO sensor (sensor_nome, moteid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        
        values[0]= mot->configs.sens4;
        res = PQexecParams(conn, "INSERT INTO sensor (sensor_nome, moteid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);

        
    }
   
    if((y==3) && (strlen(mot->configs.nome)==5)) 
    {
        res=PQexec(conn, "INSERT INTO mote (moteid) VALUES(3)");

        values[0]= mot->configs.sens1;
        values[1]= mote_conta;
        res = PQexecParams(conn, "INSERT INTO sensor (sensor_nome, moteid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);

        values[0]= mot->configs.sens2;
        res = PQexecParams(conn, "INSERT INTO sensor (sensor_nome, moteid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
    }
   
}
void escreve_atuadores(int y, PGresult *res, PGconn *conn, struct mote *mot)
{
    char*values[150];

    
    if(y==1)
    {
        
        values[0]=mot->configs.at1;
        res = PQexecParams(conn, "INSERT INTO atuador (atuador_nome) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);

        values[0]=mot->configs.at2;
        res = PQexecParams(conn, "INSERT INTO atuador (atuador_nome) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);
        
        values[0]=mot->configs.at3;
        res = PQexecParams(conn, "INSERT INTO atuador (atuador_nome) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);

        values[0]=mot->configs.at4;
        res = PQexecParams(conn, "INSERT INTO atuador (atuador_nome) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);
    }
    if(y==2 || (y==3 && (strlen(mot->configs.nome)==5)))
    {
        values[0]=mot->configs.at1;
        res = PQexecParams(conn, "INSERT INTO atuador (atuador_nome) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);

        values[0]=mot->configs.at2;
        res = PQexecParams(conn, "INSERT INTO atuador (atuador_nome) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);

    }
}

void escreve_input(int y, PGresult *res, PGconn *conn, struct mote *mot,int num)
{
    char teste[150], *values[150], ref[150];

    if(y==1 || y==2)
    {
        sprintf(teste, "%d", num);
        sprintf(ref, "%d", mot->regras[0].valor1);
        values[0]=teste;
        values[1]=mot->regras[0].comp1;
        values[2]=ref;

        res = PQexecParams(conn, "INSERT INTO inputt (inputid, comparador, referencia) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
        escreve_sensoresinput(y,res, conn, mot->regras[0].sensor1, teste, mot);
        escreve_logico(y, res, conn, mot->regras[0].logico1, 1, num);    


        sprintf(teste, "%d", num+1);
        sprintf(ref, "%d", mot->regras[0].valor2);
        values[0]=teste;
        values[1]=mot->regras[0].comp2;
        values[2]=ref;

        res = PQexecParams(conn, "INSERT INTO inputt (inputid, comparador, referencia) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
        escreve_sensoresinput(y,res, conn, mot->regras[0].sensor2, teste, mot);
        escreve_logico(y, res, conn, mot->regras[0].logico2, 1, num+1);   


        sprintf(teste, "%d", num+2);
        sprintf(ref, "%d", mot->regras[1].valor1);
        values[0]=teste;
        values[1]=mot->regras[1].comp1;
        values[2]=ref;

        res = PQexecParams(conn, "INSERT INTO inputt (inputid, comparador, referencia) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
        escreve_sensoresinput(y,res, conn, mot->regras[1].sensor1, teste, mot);
        escreve_logico(y, res, conn, mot->regras[1].logico1, 2, num+2);   

        sprintf(teste, "%d", num+3);
        sprintf(ref, "%d", mot->regras[1].valor2);
        values[0]=teste;
        values[1]=mot->regras[1].comp2;
        values[2]=ref;

        res = PQexecParams(conn, "INSERT INTO inputt (inputid, comparador, referencia) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
        escreve_sensoresinput(y,res, conn, mot->regras[1].sensor2, teste, mot);
        escreve_logico(y, res, conn, mot->regras[1].logico2, 2, num+3);   

        sprintf(teste, "%d", num+4);
        sprintf(ref, "%d", mot->regras[2].valor1);
        values[0]=teste;
        values[1]=mot->regras[2].comp1;
        values[2]=ref;

        res = PQexecParams(conn, "INSERT INTO inputt (inputid, comparador, referencia) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
        escreve_sensoresinput(y,res, conn, mot->regras[2].sensor1, teste, mot);
        escreve_logico(y, res, conn, mot->regras[2].logico1, 3, num+4);   

        sprintf(teste, "%d", num+5);
        sprintf(ref, "%d", mot->regras[3].valor1);
        values[0]=teste;
        values[1]=mot->regras[3].comp1;
        values[2]=ref;

        res = PQexecParams(conn, "INSERT INTO inputt (inputid, comparador, referencia) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
        escreve_sensoresinput(y,res, conn, mot->regras[3].sensor1, teste, mot);
        escreve_logico(y, res, conn, mot->regras[3].logico1, 4, num+5);   
        

        sprintf(teste, "%d", num+6);
        sprintf(ref, "%d", mot->regras[4].valor1);
        values[0]=teste;
        values[1]=mot->regras[4].comp1;
        values[2]=ref;

        res = PQexecParams(conn, "INSERT INTO inputt (inputid, comparador, referencia) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
        escreve_sensoresinput(y,res, conn, mot->regras[4].sensor1, teste, mot);
        escreve_logico(y, res, conn, mot->regras[4].logico1, 5, num+6);   

        sprintf(teste, "%d", num+7);
        sprintf(ref, "%d", mot->regras[5].valor1);
        values[0]=teste;
        values[1]=mot->regras[5].comp1;
        values[2]=ref;

        res = PQexecParams(conn, "INSERT INTO inputt (inputid, comparador, referencia) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
        escreve_sensoresinput(y,res, conn, mot->regras[5].sensor1, teste, mot);
        escreve_logico(y, res, conn, mot->regras[5].logico1, 6, num+7);   
    }
    if(y==3 && (strlen(mot->configs.nome)==5))
    {
        sprintf(teste, "%d", num);
        sprintf(ref, "%d", mot->regras[0].valor1);
        values[0]=teste;
        values[1]=mot->regras[0].comp1;
        values[2]=ref;

        res = PQexecParams(conn, "INSERT INTO inputt (inputid, comparador, referencia) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
        escreve_sensoresinput(y,res, conn, mot->regras[0].sensor1, teste, mot);
        escreve_logico(y, res, conn, mot->regras[0].logico1, 1, num);   

        sprintf(teste, "%d", num+1);
        sprintf(ref, "%d", mot->regras[0].valor2);
        values[0]=teste;
        values[1]=mot->regras[0].comp2;
        values[2]=ref;

        res = PQexecParams(conn, "INSERT INTO inputt (inputid, comparador, referencia) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
        escreve_sensoresinput(y,res, conn, mot->regras[0].sensor2, teste, mot);
        escreve_logico(y, res, conn, mot->regras[0].logico2, 1, num+1);   

        sprintf(teste, "%d", num+2);
        sprintf(ref, "%d", mot->regras[1].valor1);
        values[0]=teste;
        values[1]=mot->regras[1].comp1;
        values[2]=ref;

        res = PQexecParams(conn, "INSERT INTO inputt (inputid, comparador, referencia) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
        escreve_sensoresinput(y,res, conn, mot->regras[1].sensor1, teste, mot);
        escreve_logico(y, res, conn, mot->regras[1].logico1, 2, num+2);   
    }



}

void escreve_sensoresinput(int y, PGresult *res, PGconn *conn, char *sensor, char *num, struct mote *mot)
{
    char *values[150];


    values[0]=sensor;
    values[1]=num;


    res = PQexecParams(conn, "INSERT INTO sensoresinput (sensor_nome, inputid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
    

}

void escreve_regra(int y,  PGresult *res, PGconn *conn, struct mote *mot, int num)
{
    char *values[150], teste[150];
    if(y==1 || y==2)
    {
        sprintf(teste, "%d", num);
        values[0]=teste;
        res = PQexecParams(conn, "INSERT INTO regra (regranum) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);

        sprintf(teste, "%d", num+1);
        values[0]=teste;
        res = PQexecParams(conn, "INSERT INTO regra (regranum) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);

        sprintf(teste, "%d", num+2);
        values[0]=teste;
        res = PQexecParams(conn, "INSERT INTO regra (regranum) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);

        sprintf(teste, "%d", num+3);
        values[0]=teste;
        res = PQexecParams(conn, "INSERT INTO regra (regranum) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);

        sprintf(teste, "%d", num+4);
        values[0]=teste;
        res = PQexecParams(conn, "INSERT INTO regra (regranum) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);

        sprintf(teste, "%d", num+5);
        values[0]=teste;
        res = PQexecParams(conn, "INSERT INTO regra (regranum) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);
    }
    if(y==3 && (strlen(mot->configs.nome)==5))
    {
        sprintf(teste, "%d", num);
        values[0]=teste;
        res = PQexecParams(conn, "INSERT INTO regra (regranum) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);

        sprintf(teste, "%d", num+1);
        values[0]=teste;
        res = PQexecParams(conn, "INSERT INTO regra (regranum) VALUES ($1)", 1, NULL, values, NULL, NULL, 0);
    }
}

void escreve_logico(int y, PGresult *res, PGconn *conn, char *logico, int regra, int num)
{
    
    char *values[150], teste[150], teste2[150];

    if(y==2) regra=regra+6;
    if(y==3) regra=regra+12;
    if((strcmp(logico, "AND")==0) || (strcmp(logico, "OR")==0))
    {
        values[0]=logico;
        sprintf(teste, "%d", num);
        values[1]=teste;
        sprintf(teste2, "%d", regra);
        values[2]=teste2;
        res = PQexecParams(conn, "INSERT INTO logico (logicos, inputid, regranum) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
    }
    else
    {
        values[0]=NULL;
        sprintf(teste, "%d", num);
        values[1]=teste;
        sprintf(teste2, "%d", regra);
        values[2]=teste2;
        res = PQexecParams(conn, "INSERT INTO logico (logicos, inputid, regranum) VALUES ($1, $2, $3)", 3, NULL, values, NULL, NULL, 0);
       
        
    }
}

void escreve_output(int y, PGresult *res, PGconn *conn, struct mote *mot,int num)
{
    char teste[150], *values[150];
    if(y==1)
    {
        sprintf(teste, "%d", num);
        values[0]=teste;
        if(mot->regras[0].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 1, res, conn);
        escreve_outputatuador(res, conn, mot->regras[0].atuador1, teste);

        sprintf(teste, "%d", num+1);
        values[0]=teste;
        if(mot->regras[0].estado2==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 1, res, conn);
        escreve_outputatuador(res, conn, mot->regras[0].atuador2, teste);

        sprintf(teste, "%d", num+2);
        values[0]=teste;
        if(mot->regras[1].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 2, res, conn);
        escreve_outputatuador(res, conn, mot->regras[1].atuador1, teste);

        sprintf(teste, "%d", num+3);
        values[0]=teste;
        if(mot->regras[1].estado2==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 2, res, conn);
        escreve_outputatuador(res, conn, mot->regras[1].atuador2, teste);

        sprintf(teste, "%d", num+4);
        values[0]=teste;
        if(mot->regras[2].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 3, res, conn);
        escreve_outputatuador(res, conn, mot->regras[2].atuador1, teste);

        sprintf(teste, "%d", num+5);
        values[0]=teste;
        if(mot->regras[3].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 4, res, conn);
        escreve_outputatuador(res, conn, mot->regras[3].atuador1, teste);

        sprintf(teste, "%d", num+6);
        values[0]=teste;
        if(mot->regras[4].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 5, res, conn);
        escreve_outputatuador(res, conn, mot->regras[4].atuador1, teste);

        sprintf(teste, "%d", num+7);
        values[0]=teste;
        if(mot->regras[5].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 6, res, conn);
        escreve_outputatuador(res, conn, mot->regras[5].atuador1, teste);

    }

    if(y==2)
    {
        sprintf(teste, "%d", num);
        values[0]=teste;
        if(mot->regras[0].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 7, res, conn);
        escreve_outputatuador(res, conn, mot->regras[0].atuador1, teste);

        sprintf(teste, "%d", num+1);
        values[0]=teste;
        if(mot->regras[1].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 8, res, conn);
        escreve_outputatuador(res, conn, mot->regras[1].atuador1, teste);

        sprintf(teste, "%d", num+2);
        values[0]=teste;
        if(mot->regras[2].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 9, res, conn);
        escreve_outputatuador(res, conn, mot->regras[2].atuador1, teste);

        sprintf(teste, "%d", num+3);
        values[0]=teste;
        if(mot->regras[2].estado2==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 9, res, conn);
        escreve_outputatuador(res, conn, mot->regras[2].atuador2, teste);

        sprintf(teste, "%d", num+4);
        values[0]=teste;
        if(mot->regras[3].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 10, res, conn);
        escreve_outputatuador(res, conn, mot->regras[3].atuador1, teste);

        sprintf(teste, "%d", num+5);
        values[0]=teste;
        if(mot->regras[3].estado2==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 10, res, conn);
        escreve_outputatuador(res, conn, mot->regras[3].atuador2, teste);

        sprintf(teste, "%d", num+6);
        values[0]=teste;
        if(mot->regras[4].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 11, res, conn);
        escreve_outputatuador(res, conn, mot->regras[4].atuador1, teste);

        sprintf(teste, "%d", num+7);
        values[0]=teste;
        if(mot->regras[5].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 12, res, conn);
        escreve_outputatuador(res, conn, mot->regras[5].atuador1, teste);
    }
    if((y==3) && (strlen(mot->configs.nome)==5))
    {
        sprintf(teste, "%d", num);
        values[0]=teste;
        if(mot->regras[0].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 13, res, conn);
        escreve_outputatuador(res, conn, mot->regras[0].atuador1, teste);

        sprintf(teste, "%d", num+1);
        values[0]=teste;
        if(mot->regras[0].estado2==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 13, res, conn);
        escreve_outputatuador(res, conn, mot->regras[0].atuador2, teste);

        sprintf(teste, "%d", num+2);
        values[0]=teste;
        if(mot->regras[1].estado1==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 14, res, conn);
        escreve_outputatuador(res, conn, mot->regras[1].atuador1, teste);

        sprintf(teste, "%d", num+3);
        values[0]=teste;
        if(mot->regras[1].estado2==1) values[1]="ON";
        else values[1]="OFF";
        res = PQexecParams(conn, "INSERT INTO outputt (outputid, newstate_output) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_regraoutput(teste, 14, res, conn);
        escreve_outputatuador(res, conn, mot->regras[1].atuador2, teste);
    }
}
void escreve_regraoutput(char *output, int num, PGresult *res, PGconn *conn)
{
    char *values[150], teste[150];
    values[0]=output;
    sprintf(teste, "%d", num);
    values[1]=teste;
    res = PQexecParams(conn, "INSERT INTO regraoutput (outputid, regranum) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
}

void escreve_outputatuador( PGresult *res, PGconn *conn, char *sensor, char *num)
{
    char *values[150];
    values[0]=sensor;
    values[1]=num;
    res = PQexecParams(conn, "INSERT INTO outputatuador (atuador_nome, outputid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);

}

void escreve_valor(int y, struct mote *mot, PGresult *res, PGconn *conn,  char *temp, char *hum, char *luz, char *volt, char *curr, int *cont)
{
    printf("VALOR\n");
    char teste[150], *values[150], teste2[150];
    //int cont=*conte;
    printf("CONT\n");
    if(y==1)
    {
        sprintf(teste, "%f", visible_light(luz));
        values[0]=teste;
        sprintf(teste2, "%d", *cont);
        values[1]=teste2;
        res = PQexecParams(conn, "INSERT INTO valor (valor_num, valorid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_sensoresvalor(res, conn, mot->configs.sens1, teste2);

        sprintf(teste, "%f", relative_humidity(hum));
        values[0]=teste;
        sprintf(teste2, "%d", *cont+1);
        values[1]=teste2;
        res = PQexecParams(conn, "INSERT INTO valor (valor_num, valorid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_sensoresvalor(res, conn, mot->configs.sens2, teste2);
        
        sprintf(teste, "%f", temperature(temp));
        values[0]=teste;
        sprintf(teste2, "%d", *cont+2);
        values[1]=teste2;
        res = PQexecParams(conn, "INSERT INTO valor (valor_num, valorid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_sensoresvalor(res, conn, mot->configs.sens3, teste2);

        sprintf(teste, "%f", voltage(volt));
        values[0]=teste;
        sprintf(teste2, "%d", *cont+3);
        values[1]=teste2;
        res = PQexecParams(conn, "INSERT INTO valor (valor_num, valorid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_sensoresvalor(res, conn, mot->configs.sens4, teste2);

        *cont=*cont+4;
    }

    if(y==2)
    {
        sprintf(teste, "%f", temperature(temp));
        values[0]=teste;
        sprintf(teste2, "%d", *cont);
        values[1]=teste2;
        res = PQexecParams(conn, "INSERT INTO valor (valor_num, valorid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_sensoresvalor(res, conn, mot->configs.sens1, teste2);

        sprintf(teste, "%f", relative_humidity(hum));
        values[0]=teste;
        sprintf(teste2, "%d", *cont+1);
        values[1]=teste2;
        res = PQexecParams(conn, "INSERT INTO valor (valor_num, valorid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_sensoresvalor(res, conn, mot->configs.sens2, teste2);

        sprintf(teste, "%f", visible_light(luz));
        values[0]=teste;
        sprintf(teste2, "%d", *cont+2);
        values[1]=teste2;
        res = PQexecParams(conn, "INSERT INTO valor (valor_num, valorid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_sensoresvalor(res, conn, mot->configs.sens3, teste2);

        sprintf(teste, "%f", current(curr));
        values[0]=teste;
        sprintf(teste2, "%d", *cont+3);
        values[1]=teste2;
        res = PQexecParams(conn, "INSERT INTO valor (valor_num, valorid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_sensoresvalor(res, conn, mot->configs.sens4, teste2);

        *cont=*cont+4;
    }

    if((y==3) && (strlen(mot->configs.nome)==5))
    {
        sprintf(teste, "%f", voltage(volt));
        values[0]=teste;
        sprintf(teste2, "%d", *cont);
        values[1]=teste2;
        res = PQexecParams(conn, "INSERT INTO valor (valor_num, valorid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_sensoresvalor(res, conn, mot->configs.sens1, teste2);

        sprintf(teste, "%f", visible_light(luz));
        values[0]=teste;
        sprintf(teste2, "%d", *cont+1);
        values[1]=teste2;
        res = PQexecParams(conn, "INSERT INTO valor (valor_num, valorid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_sensoresvalor(res, conn, mot->configs.sens2, teste2);

        *cont=*cont+2;
    }
    
}

void escreve_sensoresvalor(PGresult *res, PGconn *conn, char *sensor, char *num)
{
    char *values[150];

    
    values[0]=sensor;
    values[1]=num;
    res = PQexecParams(conn, "INSERT INTO sensoresvalor (sensor_nome, valorid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);

}

void escreve_estado(int y, PGresult *res, PGconn *conn, struct mote *mot, int estado, char *atua, int *num)
{
    char *values[150], teste[150];
    if((y==1) || (y==2) || ((y==3) && (strlen(mot->configs.nome)==5)))
    {
        if(estado==1) values[0]="ON";
        else values[0]="OFF";
        sprintf(teste, "%d", *num);
        values[1]=teste;
        res = PQexecParams(conn, "INSERT INTO estado (newstate, estadoid) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
        escreve_atuadorestado(res, conn, teste, atua);
        *num=*num+1;
    }
}

void escreve_atuadorestado(PGresult *res, PGconn *conn, char* teste, char* atuador)
{
    char *values[150];
    values[0]=teste;
    values[1]=atuador;
    res = PQexecParams(conn, "INSERT INTO atuadorestado (estadoid, atuador_nome) VALUES ($1, $2)", 2, NULL, values, NULL, NULL, 0);
}
/*
if(PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            fprintf(stderr, "SELECT failed: %s\n", PQerrorMessage(conn));
            PQclear(res);
        }
    */
// escreve_valor(&mote1, raw_temperature, raw_humidity, raw_visible_light, raw_voltage, raw_current);