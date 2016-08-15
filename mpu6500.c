#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

#define MPU6500_WHO_AM_I	0x75
#define MPU6500_SAMPLE_DIV	0x19
#define MPU6500_CONFIG		0x1A
#define MPU6500_GYRO_CONFIG	0x1B
#define MPU6500_ACCEL_CONFIG1	0x1C
#define MPU6500_ACCEL_CONFIG2	0x1D
#define MPU6500_FIFO_EN		0x23
#define MPU6500_INT_PIN_CFG	0x37
#define MPU6500_INT_ENABLE	0x38
#define MPU6500_INT_STATUS	0x3A
#define MPU6500_ACCEL_XOUT	0x3B
#define MPU6500_ACCEL_XOUT_H	0x3B
#define MPU6500_ACCEL_XOUT_L	0x3C
#define MPU6500_ACCEL_YOUT	0x3D
#define MPU6500_ACCEL_YOUT_H	0x3D
#define MPU6500_ACCEL_YOUT_L	0x3E
#define MPU6500_ACCEL_ZOUT	0x3F
#define MPU6500_ACCEL_ZOUT_H	0x3F
#define MPU6500_ACCEL_ZOUT_L	0x40
#define MPU6500_TEMP_OUT	0x41
#define MPU6500_TEMP_OUT_H	0x41
#define MPU6500_TEMP_OUT_L	0x42
#define MPU6500_GYRO_XOUT	0x43
#define MPU6500_GYRO_XOUT_H	0x43
#define MPU6500_GYRO_XOUT_L	0x44
#define MPU6500_GYRO_YOUT	0x45
#define MPU6500_GYRO_YOUT_H	0x45
#define MPU6500_GYRO_YOUT_L	0x46
#define MPU6500_GYRO_ZOUT	0x47
#define MPU6500_GYRO_ZOUT_H	0x47
#define MPU6500_GYRO_ZOUT_L	0x48
#define MPU6500_USER_CTRL	0x6A
#define MPU6500_PWR_MGMT_1	0x6B
#define MPU6500_PWR_MGMT_2	0x6C

#define DRV_NAME "mpu6500"

struct mpu6500_data {
	struct i2c_client *client;
	uint16_t gyro_x;
	uint16_t gyro_y;
	uint16_t gyro_z;
	uint16_t accel_x;
	uint16_t accel_y;
	uint16_t accel_z;
};

static struct mpu6500_data *mpu_data;

static const struct i2c_device_id mpu6500_id[] = {
	{DRV_NAME, 0}, 
	{}
};

MODULE_DEVICE_TABLE(i2c, mpu6500_id);

static int mpu6500_write(const char reg, const char data) {
	int ret = 0;

	ret = i2c_smbus_write_byte_data(mpu_data->client, reg, data);

	if (ret < 0)
		pr_info("[%s]: Failed to write data to the device! (reg = 0x%2x, data = 0x%2x)\n", DRV_NAME, reg, data);

	return ret;
}

static int mpu6500_read(const char reg)
{
	int ret = 0;

	ret = i2c_smbus_read_byte_data(mpu_data->client, reg);

	if (ret < 0)
		pr_info("[%s]: Failed to read from device (reg = 0x%2x)!\n", DRV_NAME, reg);

	return ret;
}

static int mpu6500_read_sensor_data(const char reg, char *buff)
{
	int ret = 0;

	ret = i2c_smbus_read_i2c_block_data(mpu_data->client, reg, 2, buff);
	if(ret < 0)
		pr_info("[%s]: Failed to read from device (reg = 0x%2x)!\n", DRV_NAME, reg);

	return ret;
}

static int mpu6500_chip_init(void)
{
	int ret = 0;

	ret = mpu6500_write(MPU6500_PWR_MGMT_1, 0x01);
	if (ret < 0)
		return -EINVAL;
	ret = mpu6500_write(MPU6500_SAMPLE_DIV, 0x07);
	if (ret < 0)
		return -EINVAL;
	ret = mpu6500_write(MPU6500_CONFIG, 0x06);
	if (ret < 0)
		return -EINVAL;
	ret = mpu6500_write(MPU6500_GYRO_CONFIG, 0x10);
	if (ret < 0)
		return -EINVAL;
	ret = mpu6500_write(MPU6500_ACCEL_CONFIG1, 0x01);
	if (ret < 0)
		return -EINVAL;

	return 0;
}

static ssize_t mpu6500_gyroXYZ_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char buff[2] = {0x00};

	mpu_data->gyro_x = 0;
	mpu_data->gyro_y = 0;
	mpu_data->gyro_z = 0;
	
	mpu6500_read_sensor_data(MPU6500_GYRO_XOUT, buff);
	mpu_data->gyro_x = (mpu_data->gyro_x | buff[0]) << 16;
	mpu_data->gyro_x = (mpu_data->gyro_x | buff[1]);
	mpu6500_read_sensor_data(MPU6500_GYRO_YOUT, buff);
	mpu_data->gyro_y = (mpu_data->gyro_y | buff[0]) << 16;
	mpu_data->gyro_y = (mpu_data->gyro_y | buff[1]);
	mpu6500_read_sensor_data(MPU6500_GYRO_ZOUT, buff);
	mpu_data->gyro_z = (mpu_data->gyro_z | buff[0]) << 16;
	mpu_data->gyro_z = (mpu_data->gyro_z | buff[1]);

	return sprintf(buf, "%d (0x%x) %d (0x%x) %d(0x%x)", mpu_data->gyro_x, mpu_data->gyro_x, mpu_data->gyro_y, mpu_data->gyro_y, mpu_data->gyro_z, mpu_data->gyro_z);
}
static DEVICE_ATTR(gyroXYZ, S_IRUSR, mpu6500_gyroXYZ_show, NULL);

static ssize_t mpu6500_accelXYZ_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char buff[2] = {0x00};
	
	mpu_data->accel_x = 0;
	mpu_data->accel_y = 0;
	mpu_data->accel_z = 0;

	mpu6500_read_sensor_data(MPU6500_ACCEL_XOUT, buff);
	mpu_data->accel_x = (mpu_data->accel_x | buff[0]) << 16;
	mpu_data->accel_x = (mpu_data->accel_x | buff[1]);
	mpu6500_read_sensor_data(MPU6500_ACCEL_YOUT, buff);
	mpu_data->accel_y = (mpu_data->accel_y | buff[0]) << 16;
	mpu_data->accel_y = (mpu_data->accel_y | buff[1]);
	mpu6500_read_sensor_data(MPU6500_ACCEL_ZOUT, buff);
	mpu_data->accel_z = (mpu_data->accel_z | buff[0]) << 16;
	mpu_data->accel_z = (mpu_data->accel_z | buff[1]);

	return sprintf(buf, "%d (0x%x) %d (0x%x) %d (0x%x)", mpu_data->accel_x, mpu_data->accel_x, mpu_data->accel_y, mpu_data->accel_y, mpu_data->accel_z, mpu_data->accel_z);
}
static DEVICE_ATTR(accelXYZ, S_IRUSR, mpu6500_accelXYZ_show, NULL);

static ssize_t mpu6500_who_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint8_t who = 0;

	who = mpu6500_read(MPU6500_WHO_AM_I);

	return sprintf(buf, "0x%2x", who);
}
static DEVICE_ATTR(who, S_IRUSR, mpu6500_who_show, NULL);

static struct attribute *mpu6500_params[] = {
	&dev_attr_accelXYZ.attr,
	&dev_attr_gyroXYZ.attr,
	&dev_attr_who.attr,
	NULL,
};

static struct attribute_group mpu6500_group = {
	.name = "sensors",
	.attrs = mpu6500_params,
};

static int mpu6500_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;

	mpu_data = (struct mpu6500_data *)kmalloc(sizeof(struct mpu6500_data), GFP_KERNEL);
	if (!mpu_data) {
		pr_info("[%s]: failed to allocate memory!\n", DRV_NAME);
		return -ENOMEM;
	};

	ret = sysfs_create_group(&client->dev.kobj, &mpu6500_group);
	if (ret < 0) {
		pr_info("[%s]: failed to create sysfs group!\n", DRV_NAME);
		goto cleanup_probe_malloc;
	}

	mpu_data->client = client;

	mpu6500_chip_init();

	pr_info("[%s]: probe successful!\n", DRV_NAME);

	return 0;

cleanup_probe_malloc:
	kfree(mpu_data);
	mpu_data = NULL;

	return ret;
}

static int mpu6500_remove(struct i2c_client *client)
{
	pr_info("[%s]: removed\n", DRV_NAME);

	return 0;
}

static struct i2c_driver mpu6500_drv = {
	.probe = mpu6500_probe,
	.remove = mpu6500_remove,
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
	},
	.id_table = mpu6500_id,
};

static int __init mpu6500_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&mpu6500_drv);

	if (ret < 0) {
		pr_info("[%s]: Failed to register driver!\n", DRV_NAME);
		return -1;
	}

	return ret;
}

static void __exit mpu6500_exit(void)
{
	sysfs_remove_group(&mpu_data->client->dev.kobj, &mpu6500_group);

	if (mpu_data) {
		kfree(mpu_data);
		mpu_data = NULL;
	}

	i2c_del_driver(&mpu6500_drv);
	pr_info("[%s]: exit\n", DRV_NAME);
}

module_init(mpu6500_init);
module_exit(mpu6500_exit);

MODULE_AUTHOR("Krzysztof Garczynski");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My own MPU gyroscope driver");
