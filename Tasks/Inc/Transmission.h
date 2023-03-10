//
// Created by 14685 on 2022/7/16.
//

#ifndef HNU_RM_DOWN_TRANSMISSION_H
#define HNU_RM_DOWN_TRANSMISSION_H

#include "cmsis_os.h"

/* BCPͨѶЭ����� */
//TODO: ���ǲ�ͬ֡�������
#define FRAME_NUM     10        /* ����ͨѶ֡���������� */
#define GIMBAL_STATE_LEN         4   /* ��̨״̬���ݳ��� */
#define FRAME_MAX_LEN 40        /* ͨѶ֡����󳤶� */
#define FRAME_RPY_LEN 13        /* ŷ����rpy��ʽ���Ƴ��� */
#define FRAME_ODOM_LEN 36       /* ��̼ƿ��Ʒ�ʽ���� */
#define FRAME_IMU_LEN 40        /* imu���Ʒ�ʽ���� */
#define FRAME_CTRL_LEN 24       /* ��/���ٶȿ��Ʒ�ʽ���� */
/* Ŀ���ַ�� */
#define BROADCAST   0x00        /* �㲥 */
#define MAINFLOD    0x01        /* ��λ�� */
#define SENTRY_UP   0x02        /* �ڱ�����������̨ */
#define SENTRY_DOWN 0x03        /* �ڱ�����������̨ */
#define INFANTRY    0x04        /* ���������� */
#define ENGINEER    0x05        /* ���̻����� */
#define HERO        0x06        /* Ӣ�ۻ����� */
#define AIR         0x07        /* ���л����� */
#define RADAR       0x08        /* �״�վ */
#define GATHER      0x09        /* �Ӿ��ɼ�̨ */
#define STANDARD    0x10        /* AI������/ȫ�Զ����������� */
/* ������� */
#define CHASSIS                 0x10        /* �ٶȷ�ʽ���� */
#define CHASSIS_ODOM            0x11        /* ��̼Ʒ�ʽ���� */
#define CHASSIS_CTRL            0x12        /* ��/���ٶȷ�ʽ���� */
#define CHASSIS_IMU             0x13        /* ����imu���� */
#define GIMBAL                  0x20        /* ŷ����rpy��ʽ���� */
#define GAME_STATUS             0x30        /* ������������*/
#define ROBOT_HP                0x31        /* ������Ѫ������ */
#define ICRA_BUFF_DEBUFF_ZONE   0x32        /* ���������� */
#define GAME_MODE               0x33        /* ��������ɫ���� */
#define ROBOT_COMMAND           0x34        /* ������λ����Ϣ */
#define CLIENT_MAP_COMMAND      0x35        /* �״﷢��Ŀ��λ����Ϣ */
#define BARREL                  0x40        /* ����������� */
#define MANIFOLD_CTRL           0x50        /* ����ģʽ */
#define MODE                    0x60        /* ģʽ���� */
#define DEV_ERROR               0xE0        /* ������Ϣ */
#define HEARTBEAT               0xF0        /* �������� */


/**
  * @brief  ͨѶ֡�ṹ�� ��BCPͨѶЭ�飩 ��Ϊ���DATA���ȵ�֡�����ڽ�����ת
  */
typedef  struct
{
    uint8_t HEAD;  				    /*! ֡ͷ */
    uint8_t D_ADDR;                 /*! Ŀ���ַ */
    uint8_t ID;                     /*! ������ */
    uint8_t LEN;                    /*! ���ݳ��� */
    int8_t DATA[FRAME_MAX_LEN];     /*! �������� */
    uint8_t SC;                     /*! ��У�� */
    uint8_t AC;                     /*! ����У�� */
}__attribute__((packed)) BCPFrameTypeDef;

/**
  * @brief  ŷ���Ƿ�ʽ����ͨѶ֡�ṹ��
  */
typedef  struct
{
    uint8_t HEAD;  				    /*! ֡ͷ */
    uint8_t D_ADDR;                 /*! Ŀ���ַ */
    uint8_t ID;                     /*! ������ */
    uint8_t LEN;                    /*! ���ݳ��� */
    int8_t DATA[FRAME_RPY_LEN];     /*! �������� */
    uint8_t SC;                     /*! ��У�� */
    uint8_t AC;                     /*! ����У�� */
}__attribute__((packed)) BCPRpyTypeDef;

/**
  * @brief  imu��ʽ����ͨѶ֡�ṹ��
  */
typedef  struct
{
    uint8_t HEAD;  				    /*! ֡ͷ */
    uint8_t D_ADDR;                 /*! Ŀ���ַ */
    uint8_t ID;                     /*! ������ */
    uint8_t LEN;                    /*! ���ݳ��� */
    int8_t DATA[FRAME_IMU_LEN];    /*! �������� */
    uint8_t SC;                     /*! ��У�� */
    uint8_t AC;                     /*! ����У�� */
}__attribute__((packed)) BCPImuTypeDef;

/**
  * @brief  ��/���ٶȷ�ʽ����ͨѶ֡�ṹ��
  */
typedef  struct
{
    uint8_t HEAD;  				    /*! ֡ͷ */
    uint8_t D_ADDR;                 /*! Ŀ���ַ */
    uint8_t ID;                     /*! ������ */
    uint8_t LEN;                    /*! ���ݳ��� */
    int8_t DATA[FRAME_CTRL_LEN];    /*! �������� */
    uint8_t SC;                     /*! ��У�� */
    uint8_t AC;                     /*! ����У�� */
}__attribute__((packed)) BCPCtrlTypeDef;

/**
  * @brief  ������̨��Ϣ�ṹ��
  */
typedef  struct
{
    uint8_t HEAD;  				    /*! ֡ͷ */
    uint8_t D_ADDR;                 /*! Ŀ���ַ */
    uint8_t ID;                     /*! ������ */
    uint8_t LEN;                    /*! ���ݳ��� */
    int8_t DATA[GIMBAL_STATE_LEN];    /*! �������� */
    uint8_t SC;                     /*! ��У�� */
    uint8_t AC;                     /*! ����У�� */
}__attribute__((packed)) BCPGimStateTypeDef;

extern BCPFrameTypeDef upper_rx_data;       //������λ��������ת֡
extern BCPFrameTypeDef upper_tx_data;       //������λ��������ת֡
extern BCPFrameTypeDef upper_tx_all_data[FRAME_NUM];       //�ϲ�������λ������֡

extern BCPRpyTypeDef rpy_rx_data;           //����ŷ���Ƿ�ʽ��������֡

extern BCPImuTypeDef imu_tx_data;           //������̼Ʒ�ʽ��������֡

extern BCPCtrlTypeDef ctrl_tx_data;         //���ͽ�/���ٶȷ�ʽ��������֡
extern BCPCtrlTypeDef ctrl_rx_data;         //���ս�/���ٶȷ�ʽ��������֡

extern BCPGimStateTypeDef gim_state_data;  // ������̨״̬������֡

void Chassis_Send_supercap(void);

/**
 * @brief   ������ϵͳ�Է�������Ĺ���������͸���̨
 */
void Chassis_Send_shoot(void);

/**
 * @brief BCP��У���㷨
 * @param frame
 * @retval �ϲ�Ϊһ�����ݵ�У���� SC:��У�飨��8λ�� AC:���Ӻ�У�飨��8λ��
 */
uint16_t Sumcheck_Cal(BCPFrameTypeDef frame);

/**
 * @brief ��Ҫ���͵������������뵽��Ӧ������֡��
 * @param send_mode �������
 * @param data_buf ��װ�õ�DATA
 */
void Add_Frame_To_Upper(uint16_t send_mode, int8_t* data_buf);

#endif //HNU_RM_DOWN_TRANSMISSION_H
