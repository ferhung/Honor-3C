
#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"
#ifdef BUILD_LK
    #include <platform/disp_drv_platform.h>
#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
#else
    #include <linux/delay.h>
    #include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  		(540)
#define FRAME_HEIGHT 		(960)

#define REGFLAG_DELAY       		0xFE
#define REGFLAG_END_OF_TABLE    	0xFD   // END OF REGISTERS MARKER 
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif
//when which_lcd_modual_triple() function is called, ID0 = x, ID1 = 0 ,(ID1<<2 | ID0)=0x02
const static unsigned char LCD_MODULE_ID = 0x02;
static LCM_UTIL_FUNCS lcm_util = {0};

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)						lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)			lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg_v2(cmd, buffer, buffer_size)                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  printf(fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[128];
};
/* AUO OTM9605A LCD init code */
static struct LCM_setting_table otm9605a_auo_ini_table[] = {

{0x00,1,{0x00}},	
{0xFF,3,{0x96,0x05,0x01}},// Enable cmd
{0x00,1,{0x80}},
{0xFF,2,{0x96,0x05}},
{0x00,1,{0x80}},
{0xC1,2,{0x36,0x66}},//65Hz

{0x00,1,{0x89}},
{0xC0,1,{0x01}},

//C5B1=VDD18, LVDSVDD, GVDD_TEST_EN
{0x00,1,{0xB1}},
{0xC5,1,{0x29}},

//source output levels during porch and non-display area is GND.
{0x00,1,{0x80}},
{0xC4,1,{0x9C}},

//C1A0=Display ON pump ���� pump �]�w
{0x00,1,{0xA0}},
{0xC1,1,{0x00}},

//C5C0=disable temp sensor
{0x00,1,{0xC0}},
{0xC5,1,{0x00}},

//B0C5=prevent read error
{0x00,1,{0xC5}},
{0xB0,1,{0x03}},

////auo request
{0x00,1,{0xB4}},
{0xC0,1,{0x50}},

{0x00,1,{0x91}},
{0xC5,1,{0x79}},//VGH=+18 VGL=-12

{0x00,1,{0x00}},
{0xD8,2,{0x67,0x67}},//gvdd=4.4V

//{0x00,1,{0x00}},
//{0xD9,1,{0x19}}, //VCOM

{0x00,1,{0xA6}},
{0xB3,1,{0x0B}},

{0x00,1,{0xA7}},
{0xB3,1,{0x01}},

//panel driving mode:column inversion
//{0x00,1,{0xb4}},
//{0xc0,1,{0x50}},

{0x00,1,{0x80}},
{0xCB,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x90}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xA0}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xB0}},
{0xCB,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xC0}},
{0xCB,15,{0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xD0}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00}},

{0x00,1,{0xE0}},
{0xCB,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xF0}},
{0xCB,10,{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}},

{0x00,1,{0x80}},
{0xCC,10,{0x00,0x00,0x01,0x0D,0x0F,0x0B,0x09,0x05,0x00,0x00}},

{0x00,1,{0x90}},
{0xCC,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x0E,0x10}},

{0x00,1,{0xA0}},
{0xCC,15,{0x0C,0x0A,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xB0}},
{0xCC,10,{0x00,0x00,0x06,0x10,0x0E,0x0A,0x0C,0x02,0x00,0x00}},

{0x00,1,{0xC0}},
{0xCC,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x0F,0x0D}},

{0x00,1,{0xD0}},
{0xCC,15,{0x09,0x0B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x80}},
{0xCE,12,{0x83,0x01,0x17,0x82,0x01,0x17,0x00,0x0F,0x00,0x00,0x0F,0x00}},

{0x00,1,{0x90}},
{0xCE,14,{0x13,0xBE,0x17,0x13,0xBF,0x17,0xF0,0x00,0x00,0xF0,0x00,0x00,0x00,0x00}},

{0x00,1,{0xA0}},
{0xCE,14,{0x18,0x01,0x03,0xBC,0x00,0x17,0x00,0x18,0x00,0x03,0xBD,0x00,0x17,0x00}},

{0x00,1,{0xB0}},
{0xCE,14,{0x10,0x00,0x03,0xBE,0x00,0x17,0x00,0x10,0x01,0x03,0xBF,0x00,0x17,0x00}},

{0x00,1,{0xC0}},
{0xCE,14,{0x18,0x01,0x03,0xC0,0x00,0x06,0x12,0x18,0x00,0x03,0xC1,0x00,0x06,0x12}},

{0x00,1,{0xD0}},
{0xCE,14,{0x18,0x03,0x03,0xC2,0x00,0x06,0x12,0x18,0x02,0x03,0xC7,0x00,0x06,0x12}},

{0x00,1,{0x80}},
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},

{0x00,1,{0x90}},
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},

{0x00,1,{0xA0}},
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},

{0x00,1,{0xB0}},
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},

{0x00,1,{0xC0}},
{0xCF,10,{0x01,0x01,0x00,0x00,0x00,0x00,0x01,0x02,0x00,0x00}},
//gamma setting
{0x00,1,{0x00}},
{0xE1,16,{0x00,0x08,0x0E,0x0C,0x05,0x10,0x0C,0x0B,0x03,0x07,0x0B,0x08,0x0F,0x12,0x0B,0x02}},

{0x00,1,{0x00}},
{0xE2,16,{0x00,0x08,0x0E,0x0C,0x05,0x10,0x0C,0x0B,0x03,0x07,0x0B,0x08,0x0F,0x12,0x0B,0x02}},

//solve the problem AUO otm9605a LCD display 
{0x00,1,{0xD2}},
{0xB0,1,{0x04}},

{0x00, 1, {0x00}},
{0x11, 1, {0x00}},
{REGFLAG_DELAY, 120, {}},

{0x00, 1, {0x00}},
{0x29, 1, {0x00}},
{REGFLAG_DELAY, 40, {}},
{REGFLAG_END_OF_TABLE, 0x00, {}},

};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
    {0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 0, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 0, {0x00}},
    {REGFLAG_DELAY,  20, {}},

    // Sleep Mode On
    {0x10, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++) {

        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

            case REGFLAG_DELAY :
                if(table[i].count <= 20)
                    mdelay(table[i].count);
                else
                    msleep(table[i].count);
                break;
            case REGFLAG_END_OF_TABLE :
                break;

            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }

}
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}
static void lcm_get_params(LCM_PARAMS *params)
{
        memset(params, 0, sizeof(LCM_PARAMS));
    
        params->type   = LCM_TYPE_DSI;
        params->width  = FRAME_WIDTH;
        params->height = FRAME_HEIGHT;
        //vendor advise
        params->dsi.mode   =SYNC_EVENT_VDO_MODE; 
        // DSI
        /*Video mode setting */
        params->dsi.LANE_NUM                = LCM_TWO_LANE;

        params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;       

        params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
        params->dsi.vertical_sync_active                = 10;
        params->dsi.vertical_backporch                  = 22;
        params->dsi.vertical_frontporch                 = 22;
        params->dsi.vertical_active_line                = FRAME_HEIGHT;

        params->dsi.horizontal_sync_active              = 10;
        params->dsi.horizontal_backporch                = 40;
        params->dsi.horizontal_frontporch               = 40;
        params->dsi.horizontal_active_pixel             = FRAME_WIDTH;
        params->dsi.PLL_CLOCK =240;
	 //disable the ssc
	 params->dsi.ssc_disable = 1;
}
static void lcm_id_pin_handle(void)
{
    unsigned int ret = 0;
    ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);//ID1 is float state
    if(0 != ret)
    {
        LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");
    }
    ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_DOWN);
    if(0 != ret)
    {
        LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");
    }
}
static void lcm_init(void)
{
    lcm_util.set_gpio_mode(GPIO_DISP_LRSTB_PIN, GPIO_MODE_00);
    lcm_util.set_gpio_dir(GPIO_DISP_LRSTB_PIN, GPIO_DIR_OUT);
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    mdelay(30);  //lcm power on , reset output high , delay 30ms ,then output low
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
    msleep(60);//delay 60ms ,then output high
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    msleep(50);
    lcm_id_pin_handle();/*pull up GPIO_DISP_ID0_PIN and pull down GPIO_DISP_ID1_PIN*/
    push_table(otm9605a_auo_ini_table, sizeof(otm9605a_auo_ini_table) / sizeof(struct LCM_setting_table), 1);
    lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ONE);
    #ifdef BUILD_LK
	printf("LCD otm9605a_auo lcm_init\n");
    #else
	printk("LCD otm9605a_auo lcm_init\n");
    #endif
}
static void lcm_suspend(void)
{
#ifdef BUILD_LK
	printf("LCD otm9605a_auo lcm_suspend\n");
#else
	printk("LCD otm9605a_auo lcm_suspend\n");
#endif
    lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ZERO);
    push_table(lcm_sleep_mode_in_setting, sizeof(lcm_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
#ifdef BUILD_LK
	printf("LCD otm9605a_auo lcm_resume\n");
#else
	printk("LCD otm9605a_auo lcm_resume\n");
#endif
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
	lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ONE);
}

static unsigned int lcm_compare_id(void)
{
    unsigned char LCD_ID_value = 0;
    LCD_ID_value = which_lcd_module_triple();//when there is float state pin
    if(LCD_MODULE_ID == LCD_ID_value)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
LCM_DRIVER otm9605a_qhd_vdo_auo55_lcm_drv =
{
    .name           = "otm9605a_qhd_vdo_auo55",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id     = lcm_compare_id,
};
