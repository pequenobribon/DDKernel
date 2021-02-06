/*
 *  ddc-5.c - Ejemplo de procesamiento
 *	Asignación dinamica de numero mayor y menor de un device driver de caracter
 *	Creación de la clase para el device driver de carater
 *	Creación del archivo de dispositivo para el device driver de carater
 * 	Se agregan operaciones del driver
 * 	Se agregan las definiciones de las operaciones
 */
#include <linux/module.h>	/* Necesaria para todos los modulos 			*/
#include <linux/moduleparam.h>	/* Necesaria para el manejo de parametros 		*/
#include <linux/kernel.h>	/* Necesaria para KERN_INFO 				*/
#include <linux/fs.h>		/* Necesaria para las funciones de registro de numeros	*/
#include <linux/kdev_t.h>	/* Necesaria para las macros de registro de numeros	*/
#include <linux/device.h>	/* Necesaria para la creación de clase y dispositivo	*/
#include <linux/cdev.h>		/* Necesaria para la registrar las operaciones		*/
#include <linux/slab.h>		/* Necesaria para las funciones kmalloc y kfree		*/
#include <linux/uaccess.h>	/* Necesaria para las funciones de copia de datos	*/
#include "ventana.h"

#define MAX_SIZE	4096

MODULE_LICENSE("GPL");				/* Tipo de licencia				*/
MODULE_AUTHOR("VICTOR H GARCIA O");		/* Autor del módulo 				*/
MODULE_DESCRIPTION("Creacion de DDC");		/* Descripción de la funcionalidad del módulo 	*/
MODULE_VERSION("1.0");				/* Versión del módulo 				*/
MODULE_INFO(driver, "Character Device Driver");	/* Información personalizada del usuario	*/

dev_t dispositivo = 0;
static struct class *dev_class;
static struct device *dev_file;
static struct cdev dev_cdev;

static long int *producto;
static short int *buffer;
static long int *bufferCorr;

static void procesamiento(void);
static int driver_open	 	( struct inode *inode, struct file *file );
static int driver_release	( struct inode *inode, struct file *file );
static ssize_t driver_read	( struct file *filp, char __user *buf, size_t len, loff_t *off );
static ssize_t driver_write	( struct file *filp, const char *buf,  size_t len, loff_t *off );

static struct file_operations fops =
{
	.owner	 = THIS_MODULE,
	.open	 = driver_open,
	.read	 = driver_read,
	.write	 = driver_write,
	.release = driver_release,
};

static int driver_open( struct inode *inode, struct file *file )
{
	printk(KERN_INFO "Llamada a la operacion open del DDC \n");
//Instancia del buffer que recibe datos
	buffer = kmalloc(sizeof(short int ) *  MAX_SIZE, GFP_KERNEL );
	if( buffer == NULL )
	{
		printk(KERN_ERR "Error al asignar memoria al buffer\n");
		return -ENOMEM;
	}
//Instancia del buffer que manda datos debe ser de tipo int para que quepan los datos
    bufferCorr = kmalloc(sizeof(long int ) *  MAX_SIZE, GFP_KERNEL );
	
    if( bufferCorr == NULL )
	{
		printk(KERN_ERR "Error al asignar memoria al buffer\n");
		return -ENOMEM;
	}
//instancia del buffer para guardar el producto
    producto = kmalloc(sizeof(long int ) *  MAX_SIZE, GFP_KERNEL );
	
    if( producto == NULL )
	{
		printk(KERN_ERR "Error al asignar memoria al buffer\n");
		return -ENOMEM;
	}
	return 0;
}

static int driver_release( struct inode *inode, struct file *file )
{
	printk(KERN_INFO "Llamada a la operación realease del DDC \n");
	kfree( buffer );
    kfree(bufferCorr);
    kfree(producto);
	return 0;
}

static ssize_t driver_read( struct file *filp, char __user *buf, size_t len, loff_t *off )
{
	int ret;
	printk(KERN_INFO "Llamada a la operación read del DDC \n");
    if(*off == 0 && len > 0){
    //Código original
	 //ret = copy_to_user( buf, buffer, sizeof( short int )* MAX_SIZE );
    //Mi código
    
    ret = copy_to_user( buf, bufferCorr, sizeof(long int )* MAX_SIZE );
    
	    if( ret )
	    {
		    return -EFAULT;
	    }
		 (*off) += MAX_SIZE ;
	    return MAX_SIZE;
    }
    else
        return 0;
}

static ssize_t driver_write( struct file *filp, const char *buf,  size_t len, loff_t *off )
{
	int ret;
	register int i = 0;
	printk(KERN_INFO "Llamada a la operación write del DDC, %ld \n", len);

	ret = copy_from_user( buffer, buf, len * sizeof(short int) );
	if( ret )
	{
		return -EFAULT;
	}
	for(i = 0; i < MAX_SIZE; i++){
		printk(KERN_INFO "Muestra%d: %d \n",i, buffer[i]);
    	}
	
	for(i = 0; i < MAX_SIZE; i++){
	        printk(KERN_INFO "Hamming%d: %d \n",i, hamming[i]);
	}

	procesamiento();
	return len;
}
static void procesamiento(void){
	/*register int i;
	for(i = 0; i < MAX_SIZE; i++)
		bufferCorr[i] = buffer[i] << 1;//Multiplica a buffer * 2 haciendo el corrimiento, así es más rápido*/
//Producto entre la hamming y el seno discreto
	register int i, l, n;
	//long int producto[MAX_SIZE];
	long int proc;

	for(i = 0; i < MAX_SIZE; i++){
		proc = buffer[i] * hamming[i];
		producto[i] = (short int)( proc >> 15 );//Q0
	}

//Cálculo de la función de autocorrelación
  // register int l, n;
    for(l = 0; l < MAX_SIZE;l++){
    
    bufferCorr[l] = 0;
    
        for(n = l; n < MAX_SIZE; n++){
            bufferCorr[l] += producto[ n ] * producto[ n - l];        
        }
    }
}

static int __init funcion_inicio(void)
{
	int ret;

	printk(KERN_INFO "------------------------------------\n");
	printk(KERN_INFO "Iniciando el modulo para asignar numeros estaticos en DDC \n");
	ret = alloc_chrdev_region( &dispositivo, 0, 1, "ESCOM_dev");

	if( ret < 0 )
	{
		printk(KERN_ERR "Error al asignar el numero mayor y menor del DDC\n");
		return ret;
	}
	printk(KERN_INFO "Numeros asignados correctamente, Mayor: %d, Menor: %d \n", MAJOR(dispositivo), MINOR(dispositivo));

	cdev_init( &dev_cdev, &fops );
	ret = cdev_add( &dev_cdev, dispositivo, 1);
	if( ret < 0 )
	{
		unregister_chrdev_region( dispositivo, 1 );
		printk(KERN_ERR "Error al asignar al registrar las operaciones\n");
		return ret;
	}
	printk(KERN_INFO "Operaciones registradas correctamente \n");

	dev_class = class_create( THIS_MODULE, "ESCOM_class" );
	if( IS_ERR( dev_class ) )
	{
		printk(KERN_ERR "Error al crear la clase de dispositivo\n");
		unregister_chrdev_region( dispositivo, 1 );

		return PTR_ERR( dev_class );
	}
	printk(KERN_INFO "Clase de dispositivo creada exitosamente \n");

	dev_file = device_create( dev_class, NULL, dispositivo, NULL, "ESCOM_device");
	if( IS_ERR( dev_file) )
	{
		printk(KERN_ERR "Error al crear el archivo de dispositivo\n");
		unregister_chrdev_region( dispositivo, 1 );
		class_destroy( dev_class );

		return PTR_ERR( dev_file );
	}
	printk(KERN_INFO "Archivo de dispositivo creado exitosamente \n");

	return 0;
}

static void __exit funcion_exit(void)
{
	printk(KERN_INFO "------------------------------------\n");
	printk(KERN_INFO "Terminando el modulo para el DDC \n");

	device_destroy( dev_class, dispositivo );
	class_destroy( dev_class );
	cdev_del( &dev_cdev );
	unregister_chrdev_region( dispositivo, 1 );
}

module_init(funcion_inicio);
module_exit(funcion_exit);
