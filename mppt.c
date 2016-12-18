/*
Copyright (C) 2011 Jesus Perez <jepefe@gmail.com>
http://www.jeperez.com/modbus-y-regulador-morningstar-tristar-mppt60/

Copyright (C) 2016 Edward H. Winters <ewinters@ymdesign.net>
http://www.ymdesign.net/off-grid-energy/

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License at <http://www.gnu.org/licenses/>
for more details.
*/

/*
http://www.morningstarcorp.com/wp-content/uploads/2014/02/TSMPPT.APP_.Modbus.EN_.10.2.pdf

Install libmodbus:
wget http://github.com/downloads/stephane/libmodbus/libmodbus-3.0.1.tar.gz
tar -zxvf  libmodbus-3.0.1.tar.gz 
cd libmodbus-3.0.1
./configure --prefix=/usr 
make 
sudo make install
sudo /sbin/ldconfig

Compile: gcc mppt.c -o mppt -lmodbus 
Run: ./mppt IP PORT 
*/

#include <stdio.h>
#include <stdlib.h>
#include <modbus/modbus.h>

double valuemod(int i, uint16_t *tab);
void main(int argvc, char *argv[]) {
    modbus_t *mb;
    uint16_t tab_reg[92];
    int i=0;

    // Nombre de las variables y direccion logica segun el datasheet.
    // Name of variables and logical address according to the datasheet.
    enum var_name {
        V_PU=1,
        V_PU_LO,
        I_PU,
        I_PU_LO,
        ver_sw,
        adc_vb_f_med=24,
        adc_vbterm_f,
        adc_vbs_f,
        adc_va_f,
        adc_ib_f_shadow,
        adc_ia_f_shadow,
        adc_p12_f,
        adc_p3_f,
        adc_pmeter_f,
        adc_p18_f,
        adc_v_ref,
        T_hs,
        T_rts,
        T_batt,
        adc_vb_f_lm,
        adc_ib_f_lm,
        vb_min,
        vb_max,
        hourmeter_HI,
        hourmeter_LO,
        faultall,
        alarm_HI=46,
        alarm_LO,
        dip_all,
        led_state,
        charge_state,
        vb_ref,
        ahc_r_HI,
        ahc_r_LO,
        ahc_t_HI,
        ahc_t_LO,
        kwhc_r,
        kwhc_t,
        power_out_shadow,
        power_in_shadow,
        sweep_Pin_max,
        sweep_vmp,
        sweep_voc,
        vb_min_daily=64,
        vb_max_daily,
        va_max_daily,
        ahc_daily,
        whc_daily,
        flags_daily,
        pout_max_daily,
        tb_min_daily,
        tb_max_daily,
        fault_daily,
        alarm_daily_hi=75,
        alarm_daily_lo,
        time_ab_daily,
        time_eq_daily,
        time_fl_daily,
        vb_ref_slave=90,
        va_ref_fixed,
        va_ref_fixed_pct
    };

    // Nombres de las variables para mostrar.
    // Variable names.
    char *var_label[]= {
        "V_PU",
        "V_PU_LO",
        "I_PU",
        "I_PU_LO",
        "VER_SW",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "ADC_VB_F_MED",
        "ADC_VBTERM_F",
        "ADC_VBS_F",
        "ADC_VA_F",
        "ADC_IB_F_SHADOW",
        "ADC_IA_F_SHADOW",
        "ADC_P12_F",
        "ADC_P3_F",
        "ADC_PMETER_F",
        "ADC_P18_F",
        "ADC_V_REF",
        "T_HS",
        "T_RTS",
        "T_BATT",
        "ADC_VB_F_LM",
        "ADC_IB_F_LM",
        "VB_MIN",
        "VB_MAX",
        "HOURMETER_HI",
        "HOURMETER_LO",
        "FAULTALL",
        "RESV",
        "ALARM_HI",
        "ALARM_LO",
        "DIP_ALL",
        "LED_STATE",
        "CHARGE_STATE",
        "VB_REF",
        "AHC_R_HI",
        "AHC_R_LO",
        "AHC_T_HI",
        "AHC_T_LO",
        "KWHC_R",
        "KWHC_T",
        "POWER_OUT_SHADOW",
        "POWER_IN_SHADOW",
        "SWEEP_PIN_MAX",
        "SWEEP_VMP",
        "SWEEP_VOC",
        "RESV",
        "VB_MIN_DAILY",
        "VB_MAX_DAILY",
        "VA_MAX_DAILY",
        "AHC_DAILY",
        "WHC_DAILY",
        "FLAGS_DAILY",
        "POUT_MAX_DAILY",
        "TB_MIN_DAILY",
        "TB_MAX_DAILY",
        "FAULT_DAILY",
        "RESV",
        "ALARM_DAILY_HI",
        "ALARM_DAILY_LO",
        "TIME_AB_DAILY",
        "TIME_EQ_DAILY",
        "TIME_FL_DAILY",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "RESV",
        "VB_REF_SLAVE",
        "VA_REF_FIXED",
        "VA_REF_FIXED_PCT"
    };

    // Creamos una nueva conexion tcp indicando IP y puerto.
    // Create a new tcp connection indicating IP address and port.
    mb = modbus_new_tcp(argv[1], atoi(argv[2]));
    modbus_set_slave(mb,1);

    // Conectamos con el dispositivo.
    // Connect to thee device.
    modbus_connect(mb);
    if ( mb == NULL ) printf("Error al conectar\n");

    // Leemos los registros desde el 0 al 91
    // Read registers 0 through 91.
    modbus_read_registers(mb, 0,91, tab_reg);

    // Mostramos datos realizando las operaciones que nos indica el fabricante.
    // Show data by performing the operations indicated by the manufacturer.
    for(i=0;i<92;i++){
	if (var_label[i] != "RESV") {
            // printf("%s:%.3f\n",var_label[i],valuemod(i,tab_reg));
            printf("%s:%.3f ",var_label[i],valuemod(i,tab_reg));
	}
    }
    modbus_close(mb);
    modbus_free(mb);
}

double valuemod(int id, uint16_t *tab){
// Dependiendo de la direccion del registro realizamos las operaciones correspondientes.
// Depending on the register address, perform the corresponding operation.
        double val;
        switch(id){
                case 24:
                case 25:
                case 26:
                case 27:
                case 38:
                case 40:
                case 41:
                case 51:
                case 61:
                case 62:
                case 64:
                case 65:
                case 66:
                case 89:
                case 90:
                    val = tab[id]*tab[0]*0.000030518;
                    break;
		case 30:
		case 32:
		    val = tab[id]*18.612*0.000030518;
		    break;
                case 28:
                case 29:
		case 39:
                    val = tab[id]*tab[2]*0.000030518;
                    break;
		case 31:
		    val = tab[id]*6.6*0.000030518;
		    break;
		case 33:
		case 34:
		    val = tab[id]*3*0.000030518;
		    break;
		case 35: 
		case 36:
		case 37:
		case 71:
		//case 72:
		    val = tab[id]-65536;
		    break;
                case 58:
                case 59:
                case 60:
                case 70:
                    val = tab[id]*tab[0]*tab[2]*0.0000076294;
                    break;
		case 52:
		case 54:
		case 67:
		    val= tab[id]*0.1;
		    break;
		case 91:
		    val = tab[id]*100*0.000030518;
		    break;
                default:
                    val=tab[id];
        }
return val;
}
