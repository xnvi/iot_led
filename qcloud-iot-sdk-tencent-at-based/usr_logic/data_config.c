/*-----------------data config start  -------------------*/ 

#define TOTAL_PROPERTY_COUNT 9

static sDataPoint    sg_DataTemplate[TOTAL_PROPERTY_COUNT];

typedef struct _ProductDataDefine {
    TYPE_DEF_TEMPLATE_BOOL m_power_switch;
    TYPE_DEF_TEMPLATE_STRING m_name[64+1];
    TYPE_DEF_TEMPLATE_INT m_color_temp;
    TYPE_DEF_TEMPLATE_INT m_brightness;
    TYPE_DEF_TEMPLATE_INT m_red;
    TYPE_DEF_TEMPLATE_INT m_green;
    TYPE_DEF_TEMPLATE_INT m_blue;
    TYPE_DEF_TEMPLATE_INT m_colorful_mode;
    TYPE_DEF_TEMPLATE_ENUM m_default_color_mode;
} ProductDataDefine;

static   ProductDataDefine     sg_ProductData;

static void _init_data_template(void)
{
    sg_ProductData.m_power_switch = 0;
    sg_DataTemplate[0].data_property.data = &sg_ProductData.m_power_switch;
    sg_DataTemplate[0].data_property.key  = "power_switch";
    sg_DataTemplate[0].data_property.type = TYPE_TEMPLATE_BOOL;

    sg_ProductData.m_name[0] = '\0';
    sg_DataTemplate[1].data_property.data = sg_ProductData.m_name;
    sg_DataTemplate[1].data_property.data_buff_len = sizeof(sg_ProductData.m_name)/sizeof(sg_ProductData.m_name[1]);
    sg_DataTemplate[1].data_property.key  = "name";
    sg_DataTemplate[1].data_property.type = TYPE_TEMPLATE_STRING;

    sg_ProductData.m_color_temp = 5500;
    sg_DataTemplate[2].data_property.data = &sg_ProductData.m_color_temp;
    sg_DataTemplate[2].data_property.key  = "color_temp";
    sg_DataTemplate[2].data_property.type = TYPE_TEMPLATE_INT;

    sg_ProductData.m_brightness = 50;
    sg_DataTemplate[3].data_property.data = &sg_ProductData.m_brightness;
    sg_DataTemplate[3].data_property.key  = "brightness";
    sg_DataTemplate[3].data_property.type = TYPE_TEMPLATE_INT;

    sg_ProductData.m_red = 0;
    sg_DataTemplate[4].data_property.data = &sg_ProductData.m_red;
    sg_DataTemplate[4].data_property.key  = "red";
    sg_DataTemplate[4].data_property.type = TYPE_TEMPLATE_INT;

    sg_ProductData.m_green = 0;
    sg_DataTemplate[5].data_property.data = &sg_ProductData.m_green;
    sg_DataTemplate[5].data_property.key  = "green";
    sg_DataTemplate[5].data_property.type = TYPE_TEMPLATE_INT;

    sg_ProductData.m_blue = 0;
    sg_DataTemplate[6].data_property.data = &sg_ProductData.m_blue;
    sg_DataTemplate[6].data_property.key  = "blue";
    sg_DataTemplate[6].data_property.type = TYPE_TEMPLATE_INT;

    sg_ProductData.m_colorful_mode = 0;
    sg_DataTemplate[7].data_property.data = &sg_ProductData.m_colorful_mode;
    sg_DataTemplate[7].data_property.key  = "colorful_mode";
    sg_DataTemplate[7].data_property.type = TYPE_TEMPLATE_INT;

    sg_ProductData.m_default_color_mode = 0;
    sg_DataTemplate[8].data_property.data = &sg_ProductData.m_default_color_mode;
    sg_DataTemplate[8].data_property.key  = "default_color_mode";
    sg_DataTemplate[8].data_property.type = TYPE_TEMPLATE_ENUM;

};
