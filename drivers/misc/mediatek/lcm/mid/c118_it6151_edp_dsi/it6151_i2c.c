#ifndef BUILD_LK
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
//#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
//#include <cust_acc.h>
//#include <linux/hwmsensor.h>
//#include <linux/hwmsen_dev.h>
//#include <linux/sensors_io.h>
//#include <linux/hwmsen_helper.h>
//#include <linux/xlog.h>
//#include <mach/mt_typedefs.h>
//#include <mach/mt_gpio.h>
//#include <mach/mt_pm_ldo.h>
#include <mt-plat/mtk_thermal_typedefs.h>
/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define IT6151_EDP_SLAVE_ADDR_WRITE						(0x5C << 0)
#define	IT6151_MIPIRX_SLAVE_ADDR_WRITE 				(0x6C << 0)

#define DEVICE_NAME "IT6151"
//#define IT6151_DEBUG

static struct i2c_client *it6151_mipirx = NULL;
static struct i2c_client *it6151_edp = NULL;

static const struct i2c_device_id it6151_i2c_id[] = 
{
	{"it6151_edp",		1},
	{"it6151_mipirx",	1},
};

static int it6151_i2c_driver_probe(struct i2c_client *client, const struct i2c_device_id *id);

static struct i2c_driver it6151_i2c_driver = {
  .driver = {
      .name    = DEVICE_NAME,
  },
  .probe       = it6151_i2c_driver_probe,
  .id_table    = it6151_i2c_id,
};

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
static DEFINE_MUTEX(it6151_i2c_access);
/**********************************************************
  *
  *   [I2C Function For Read/Write fan5405] 
  *
  *********************************************************/
int it6151_i2c_read_byte(u8 dev_addr, u8 addr, u8 *returnData)
{
	int err;
	struct i2c_msg msgs[2] = { {0}, {0} };
	msgs[0].addr = dev_addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &addr;
	msgs[1].addr = dev_addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = 1;
	msgs[1].buf = returnData;
	
if(dev_addr == IT6151_MIPIRX_SLAVE_ADDR_WRITE){	
	err = i2c_transfer(it6151_mipirx->adapter, msgs, sizeof(msgs) / sizeof(msgs[0]));
}else{
	err = i2c_transfer(it6151_edp->adapter, msgs, sizeof(msgs) / sizeof(msgs[0]));
}
	if (err < 0) {
		printk("=====================  it6151_i2c_read_byte  i2c_transfer  err \r\n");
		err = -EIO;
	} else {
		err = 0;
	}

  return err;
}

int it6151_i2c_write_byte(u8 dev_addr, u8 addr, u8 writeData)
{
	int err;
	struct i2c_msg msgs[1] = { {0} };
	u8 buffer[2] = {0};
	u8 lens = 0;

	buffer[0] = addr;
	buffer[1] = writeData;		
	lens = 2;	

	msgs[0].addr = dev_addr;
	msgs[0].flags = 0;
	msgs[0].len = lens;
	msgs[0].buf = buffer;
	//msleep(2);

#ifdef IT6151_DEBUG
  /* dump write_data for check */
	printk("[KE/it6151_i2c_write] dev_addr = 0x%x, write_data[0x%x] = 0x%x \n", dev_addr, addr, writeData);
#endif
if(dev_addr == IT6151_MIPIRX_SLAVE_ADDR_WRITE){		
	err = i2c_transfer(it6151_mipirx->adapter, msgs, sizeof(msgs) / sizeof(msgs[0]));
}else  if(dev_addr == IT6151_EDP_SLAVE_ADDR_WRITE){
	err = i2c_transfer(it6151_edp->adapter, msgs, sizeof(msgs) / sizeof(msgs[0]));
}else{
	printk("it6151_i2c_write_byte i2c_transfer error\n");
	return 1;
}	

  return 1;
}

static int match_id(const struct i2c_client *client, const struct i2c_device_id *id)
{
	if (strcmp(client->name, id->name) == 0)
		return true;
	else
		return false;
}

static int it6151_i2c_driver_probe(struct i2c_client *client, const struct i2c_device_id *id) 
{
  int err=0; 

	printk("[it6151_i2c_driver_probe] start!\n");

	if(match_id(client, &it6151_i2c_id[0]))
	{
	  if (!(it6151_mipirx = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) 
		{
	    err = -ENOMEM;
	    goto exit;
	  }
		
	  memset(it6151_mipirx, 0, sizeof(struct i2c_client));

	  it6151_mipirx = client;    
	}
	else if(match_id(client, &it6151_i2c_id[1]))
	{
		if (!(it6151_edp = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) 
		{
			err = -ENOMEM;
			goto exit;
		}
		
		memset(it6151_edp, 0, sizeof(struct i2c_client));
	
		it6151_edp = client; 	 
	}
	else
	{
		printk("[it6151_i2c_driver_probe] error!\n");

		err = -EIO;
		goto exit;
	}

	printk("[it6151_i2c_driver_probe] %s i2c sucess!\n", client->name);
	
  return 0;

exit:
  return err;

}

#define IT6151_BUSNUM 0

static struct i2c_board_info __initdata it6151_I2C[] = 
{
	{I2C_BOARD_INFO("it6151_edp", 		IT6151_EDP_SLAVE_ADDR_WRITE)},
	{I2C_BOARD_INFO("it6151_mipirx", 	IT6151_MIPIRX_SLAVE_ADDR_WRITE)},
};

static int __init it6151_init(void)
{    
  printk("[it6151_init] init start\n");
  
  i2c_register_board_info(IT6151_BUSNUM, it6151_I2C, 2);

  if(i2c_add_driver(&it6151_i2c_driver)!=0)
  {
    printk("[it6151_init] failed to register it6151 i2c driver.\n");
  }
  else
  {
    printk("[it6151_init] Success to register it6151 i2c driver.\n");
  }

  return 0;
}

static void __exit it6151_exit(void)
{
  i2c_del_driver(&it6151_i2c_driver);
}

module_init(it6151_init);
module_exit(it6151_exit);
   
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("I2C it6151 Driver");
MODULE_AUTHOR("James Lo<james.lo@mediatek.com>");
#endif
