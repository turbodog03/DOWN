//
// Created by 14685 on 2022/7/16.
//

#include "Chassis.h"
#include "bsp_uart.h"
#include "motor.h"
#include "bsp_can.h"
#include "keyboard.h"
#include "Transmission.h"

float chassis_vx_test;
float chassis_vy_test;
float chassis_vw_test;

ChassisTypeDef chassis;
MotorTypeDef chassis_motor[4];
//���̸�����̨PID�ṹ��
PIDTypeDef rotate_follow;
/* ���̵������ת��(rpm) */
int16_t chassis_moto_speed_ref[4];
/* ������õ��ĵ��̵��ʵ���ٶ� */
float chassis_vx = 0;
float chassis_vy = 0;
float chassis_vw = 0;
/* ���̵������ */
int16_t chassis_moto_current[4];

//��������ת��ϵ��
char spin_flag=0;
float yaw_relative_angle=0;

unsigned char recv_flag = 0;   //���⴮�ڽ��ձ�־λ


void Chassis_Task(void const * argument){

    Chassis_Init_param();
    chassis.ctrl_mode = CHASSIS_STOP;
    uint32_t chassis_wake_time = osKernelSysTick();

    while (1){
        //����״̬���л�����ģʽ
        Chassis_Get_mode();
        Chassis_Get_speed();

        switch (chassis.ctrl_mode) {
            case CHASSIS_FOLLOW_GIMBAL:{
                Chassis_Follow_control();
            }break;

            case CHASSIS_OPEN_LOOP:{
                Chassis_Open_control();
            }break;

            case CHASSIS_SPIN:{
                Chassis_Spin_control();
            }break;

            case CHASSIS_FLY:{
                Chassis_Fly_control();
            }break;

            case CHASSIS_STOP:{
                Chassis_Stop_control();
            }break;

            case CHASSIS_RELAX:{
                Chassis_Relax_control();
            }break;

            default:
                break;
        }
        osDelayUntil(&chassis_wake_time, CHASSIS_PERIOD);
    }
}

void Chassis_Init_param(void)
{
    //�����������
    //osThreadSuspend(NULL);
    //����PID��������
    for (int i = 0; i < 4; i++)
    {
        PID_Init(
                &chassis_motor[i].PID_Velocity, 16000, 8000, 1, 8, 15, 0, 500, 100, 0.001, 0, 1, Integral_Limit | OutputFilter);
        chassis_motor[i].Max_Out = 8000;
    }
    //���̸���PID��������
    PID_Init(&rotate_follow, 1500, 300, 0, 10, 10, 0, 300,
             100, 0, 0.01, 5,
             Integral_Limit | Derivative_On_Measurement | Trapezoid_Intergral | DerivativeFilter);
}

void Chassis_Get_mode(void){
    chassis.last_mode = chassis.ctrl_mode;

    /* ��̨ʧ�ܣ�����Ҳ�ø���ʧ�� */
    if (gim_state_data.DATA[0] == 1) {
        chassis.ctrl_mode = CHASSIS_RELAX;
    }

    switch (chassis.ctrl_mode) {
        case CHASSIS_FOLLOW_GIMBAL:
            if (rc.kb.bit.E){
                chassis.ctrl_mode = CHASSIS_SPIN;
            }
            if (rc.kb.bit.SHIFT && rc.kb.bit.W){
                chassis.ctrl_mode = CHASSIS_FLY;
            }
            if (/*(rc.sw2 == RC_MI) || */rc.mouse.r){
                chassis.ctrl_mode = CHASSIS_OPEN_LOOP;
            }break;
        case CHASSIS_SPIN:
            if (spin_flag==0){
                chassis.ctrl_mode = CHASSIS_FOLLOW_GIMBAL;
            }break;
        case CHASSIS_FLY:
            if ((rc.kb.bit.SHIFT & rc.kb.bit.W) != 1){
                chassis.ctrl_mode = CHASSIS_FOLLOW_GIMBAL;
            }break;
        case CHASSIS_OPEN_LOOP:
            if (rc.kb.bit.E){
                chassis.ctrl_mode = CHASSIS_SPIN;
            }
            if((rc.sw2 != RC_MI) && (rc.mouse.r != 1)){
                chassis.ctrl_mode = CHASSIS_FOLLOW_GIMBAL;
            }break;
        case CHASSIS_STOP:
            if ((rc.sw1 == RC_UP) || (rc.sw1 == RC_MI)){
                chassis.ctrl_mode = CHASSIS_FOLLOW_GIMBAL;
            }break;
        case CHASSIS_RELAX:
            if (rc.sw2 != RC_DN){
                chassis.ctrl_mode = CHASSIS_FOLLOW_GIMBAL;
            }break;
    }
    if (rc.sw1 == RC_DN){
        chassis.ctrl_mode = CHASSIS_STOP;
    }
    if (rc.sw2 == RC_DN){
        chassis.ctrl_mode = CHASSIS_RELAX;
    }
}

void Chassis_Relax_control(void){
    static uint16_t data[8];
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    data[3] = 0;
    data[4] = 0;
    data[5] = 0;
    data[6] = 0;
    data[7] = 0;
    Chassis_Send_current(data);
}

void Chassis_Stop_control(void){
    chassis.vx = 0;
    chassis.vy = 0;
    chassis.vw = 0;
    Chassis_Custom_control();
}

void Chassis_Fly_control(void){
    int16_t   chassis_fly_current[]={-200, 200, 200, -200}; //���̷���ģʽʱ�����е������ֵ���16384
    chassis.vx = 0;
    chassis.vy = 0;
    chassis.vw = PID_Calculate(&rotate_follow, yaw_relative_angle,0);
    Chassis_Custom_control();
    if(yaw_relative_angle<=0.3){   //���̺���̨��׼���  //TODO
        Chassis_Send_current(chassis_fly_current);  //���ĸ���������������͸����
    }
}

void Chassis_Open_control(void){
    Chassis_Get_control_information();
    chassis.vw = 0;  //TODO
    Chassis_Custom_control();
}

void Chassis_Follow_control(void){
    Chassis_Get_control_information();
    chassis.vw = PID_Calculate(&rotate_follow, yaw_relative_angle,0);
    if ((abs(rc.ch1) <= 10)
        && (abs(rc.ch2) <= 10)
        && (abs(rc.ch3) <= 10)
        && (abs(rc.ch4) <= 10)
        && (abs(rc.mouse.x) <= 1)
        && (abs(rc.mouse.y) <= 1)){
        chassis.vw = 0;
    }
    Chassis_Top_handle();
    if(rc.sw2 == RC_MI && recv_flag){
        chassis_vx_test = (int32_t) (ctrl_rx_data.DATA[3] << 24 | ctrl_rx_data.DATA[2] << 16
                                | ctrl_rx_data.DATA[1] << 8 | ctrl_rx_data.DATA[0]);
        chassis_vy_test = (int32_t) (ctrl_rx_data.DATA[7] << 24 | ctrl_rx_data.DATA[6] << 16
                                | ctrl_rx_data.DATA[5] << 8 | ctrl_rx_data.DATA[4]);
        chassis_vw_test = (int32_t) (ctrl_rx_data.DATA[23] << 24 | ctrl_rx_data.DATA[22] << 16
                                | ctrl_rx_data.DATA[21] << 8 | ctrl_rx_data.DATA[20]) /10000;
        recv_flag = 0;
    }
    Chassis_Custom_control();
}

void Chassis_Spin_control(void){
    Chassis_Get_control_information();
    if(rc.kb.bit.E)
    spin_flag++;
    if(rc.kb.bit.E && rc.kb.bit.SHIFT)
        spin_flag=0;
    chassis.vw=10*spin_flag;   //������ת�ٶ�
    Chassis_Top_handle();
    Chassis_Custom_control();
}


void Chassis_Send_current(int16_t current[]){
    static uint8_t data[8];
    data[0] = current[0] >> 8;
    data[1] = current[0];
    data[2] = current[1] >> 8;
    data[3] = current[1];
    data[4] = current[2] >> 8;
    data[5] = current[2];
    data[6] = current[3] >> 8;
    data[7] = current[3];
    CAN_Send(CHASSIS_CAN, CAN_CHASSIS_ID, data);
}

/* ���̿����źŻ�ȡ */
void Chassis_Get_control_information(void){
    pc_kb_hook();
    //ң�����Լ����Ե��̵Ŀ�����Ϣת��Ϊ��׼��λ��ƽ��Ϊ(mm/s)��תΪ(degree/s)
    chassis.vx = rc.ch1 * CHASSIS_RC_MOVE_RATIO_X / RC_MAX_VALUE * MAX_CHASSIS_VX_SPEED + km.vx * CHASSIS_PC_MOVE_RATIO_X;
    chassis.vy = rc.ch2 * CHASSIS_RC_MOVE_RATIO_Y / RC_MAX_VALUE * MAX_CHASSIS_VY_SPEED + km.vy * CHASSIS_PC_MOVE_RATIO_Y;
    chassis.vw = rc.ch3 * CHASSIS_RC_MOVE_RATIO_R / RC_MAX_VALUE * MAX_CHASSIS_VR_SPEED + rc.mouse.x * CHASSIS_PC_MOVE_RATIO_R;
}


/* �����˶����ٶȷֽ⣬�Լ����ת�ٵıջ����� */
void Chassis_Custom_control(void){
    //�����ٶȷֽ⣬������̵��ת��
    Chassis_Calc_moto_speed(chassis.vx, chassis.vy, chassis.vw, chassis_moto_speed_ref);
    //�ջ�����������ӵ������
    Chassis_Calculate_close_loop();
    //������õĵ���ֵ���͸����
    Chassis_Send_current(chassis_moto_current);
}

/* ���̵��˶��ֽ⴦�� */
void Chassis_Calc_moto_speed(float vx, float vy, float vw, int16_t speed[]){
    static float rotate_ratio_f = ((WHEELBASE+WHEELTRACK)/2.0f - GIMBAL_OFFSET)/RADIAN_COEF;
    static float rotate_ratio_b = ((WHEELBASE+WHEELTRACK)/2.0f + GIMBAL_OFFSET)/RADIAN_COEF;
    static float wheel_rpm_ratio = 60.0f/(PERIMETER*CHASSIS_DECELE_RATIO);

    int16_t wheel_rpm[4];
    float max = 0;

    //���Ƶ��̸������ٶ�
    VAL_LIMIT(vx, -MAX_CHASSIS_VX_SPEED, MAX_CHASSIS_VX_SPEED);  //mm/s
    VAL_LIMIT(vy, -MAX_CHASSIS_VY_SPEED, MAX_CHASSIS_VY_SPEED);  //mm/s
    VAL_LIMIT(vw, -MAX_CHASSIS_VR_SPEED, MAX_CHASSIS_VR_SPEED);  //deg/s

    wheel_rpm[0] = (+vx - vy + vw * rotate_ratio_f) * wheel_rpm_ratio;
    wheel_rpm[1] = (+vx + vy + vw * rotate_ratio_f) * wheel_rpm_ratio;
    wheel_rpm[2] = (-vx + vy + vw * rotate_ratio_b) * wheel_rpm_ratio;
    wheel_rpm[3] = (-vx - vy + vw * rotate_ratio_b) * wheel_rpm_ratio;

    //����ÿ���ֵ�ת�٣��ҳ����ֵ
    for (uint8_t i = 0; i < 4; i++){
        if (abs(wheel_rpm[i]) > max)
            max = abs(wheel_rpm[i]);
    }
    //�����٣���ÿ���ֵ��ٶȳ�����ͬ����

    if (max > MAX_WHEEL_RPM){
        float rate = MAX_WHEEL_RPM / max;
        for (uint8_t i = 0; i < 4; i++)
            wheel_rpm[i] *= rate;
    }

#ifdef POWER_CONTORL
    //��������
	//�����幦�ʼ�С�ĳ̶ȸ�ÿ���ֵ��ٶȳ�����ͬ����
	if(chassis_power > POWER_LIMIT)
	{
		float r = power_buffer * power_buffer/3600;
		for(int i=0; i<4; i++){
			wheel_rpm[i] *= r;
		}
	}
#endif
    memcpy(speed, wheel_rpm, 4*sizeof(int16_t));
}

/* ���̵��˶����������ʵ���ٶ� */
void Chassis_Get_speed(void){
    static float rotate_ratio_f = ((WHEELBASE+WHEELTRACK)/2.0f - GIMBAL_OFFSET)/RADIAN_COEF;
    static float rotate_ratio_b = ((WHEELBASE+WHEELTRACK)/2.0f + GIMBAL_OFFSET)/RADIAN_COEF;
    static float wheel_rpm_ratio = 60.0f/(PERIMETER*CHASSIS_DECELE_RATIO);
    int16_t wheel_rpm[4];

    for (int i = 0; i < 4; i++)
    {
        wheel_rpm[i] = chassis_motor[i].Velocity_RPM;
    }

    chassis_vw = (wheel_rpm[0] + wheel_rpm[1] + wheel_rpm[2] + wheel_rpm[3])/
            (2 * wheel_rpm_ratio * (rotate_ratio_b + rotate_ratio_f));
    chassis_vx =  (wheel_rpm[0] + wheel_rpm[1])/
            (2 * wheel_rpm_ratio) - chassis_vw * rotate_ratio_f;
    chassis_vy = (wheel_rpm[1] + wheel_rpm[2])/
                 (2 * wheel_rpm_ratio) - (chassis_vw * (rotate_ratio_b + rotate_ratio_f))/2;
}

void Chassis_Calculate_close_loop(void){
    for (int i = 0; i < 4; i++)
    {
        chassis_moto_current[i]=Motor_Speed_Calculate(&chassis_motor[i], chassis_motor[i].Velocity_RPM, chassis_moto_speed_ref[i]);
    }
}

/* �������ݴ��� */
void Chassis_Top_handle(void){
    float rad=-(yaw_relative_angle/RADIAN_COEF);
    float a=cosf(rad),b=sinf(rad);
    float temp_x=a*chassis.vx+b*chassis.vy;
    float temp_y=-b*chassis.vx+a*chassis.vy;
    chassis.vx=temp_x;
    chassis.vy=temp_y;
}
