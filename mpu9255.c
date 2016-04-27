#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include "mpu9255.h"

#define DRV_NAME "mpu9255"

struct mpu9255_data {
	struct i2c_client *client;
	unsigned int x;
	unsigned int y;
	unsigned int z;
};

static struct mpu9255_data *mpu_data;

static const struct i2c_device_id mpu9255_id[] = {
	{DRV_NAME, 0},
	{ }
};

MODULE_DEVICE_TABLE(i2c, mpu9255_id);

static int mpu9255_write(const char reg, const char data)
{
	int ret = 0;

	ret = i2c_smbus_write_byte_data(mpu_data->client, reg, data);
	if(ret < 0)
		pr_info("[%s]: Failed to write data to the device! (reg = 0x%2x, data = 0x%2x)\n", DRV_NAME, reg, data);

	return ret;
}

static int mpu9255_read(const char reg)
{
	int ret = 0;

	ret = i2c_smbus_read_byte_data(mpu_data->client, reg);
	if(ret < 0)
		pr_info("[%s]: Failed to read from device (reg = 0x%2x)!\n", DRV_NAME, reg);

	return ret;
}

static int mpu9255_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int who = 0;

	mpu_data = (struct mpu9255_data *)kmalloc(sizeof(struct mpu9255_data), GFP_KERNEL);
	if(!mpu_data) {
		pr_info("[%s]: failed to allocate memory!\n", DRV_NAME);
		return -ENOMEM;
	};

	mpu_data->client = client;

	pr_info("[%s]: client: name = %s, addr = 0x%2x, ", DRV_NAME, client->name, client->addr);
	pr_info("[%s]: probe successful!\n", DRV_NAME);

	who = mpu9255_read(MPU9255_WHO_AM_I);
	pr_info("[%s]: Who Am I: 0x%2x\n", DRV_NAME, who);

	return 0;
}

static int mpu9255_remove(struct i2c_client *client)
{
	pr_info("[%s]: removed\n", DRV_NAME);
	return 0;
}

static struct i2c_driver mpu9255_drv = {
	.probe = mpu9255_probe,
	.remove = mpu9255_remove,
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
	},
	.id_table = mpu9255_id,
};

static int __init mpu9255_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&mpu9255_drv);

	if(ret < 0)
		pr_info("[%s]: Failed to register driver!\n", DRV_NAME);
	else
		pr_info("[%s]: initalized\n", DRV_NAME);

	return ret;
}

static void __exit mpu9255_exit(void)
{
	if(mpu_data) {
		kfree(mpu_data);
		mpu_data = NULL;
	}

	i2c_del_driver(&mpu9255_drv);

	pr_info("[%s]: exit\n", DRV_NAME);
}

module_init(mpu9255_init);
module_exit(mpu9255_exit);

MODULE_AUTHOR("Krzysztof Garczynski");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My own MPU gyroscope driver");
