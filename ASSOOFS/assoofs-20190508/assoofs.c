#include <linux/module.h>       /* Needed by all modules */
#include <linux/kernel.h>       /* Needed for KERN_INFO  */
#include <linux/init.h>         /* Needed for the macros */
#include <linux/fs.h>           /* libfs stuff           */
#include <linux/buffer_head.h>  /* buffer_head           */
#include <linux/slab.h>         /* kmem_cache            */
#include "assoofs.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("ifranl00");
/****** declarcion de funciones ******/

struct assoofs_inode_info *assoofs_get_inode_info(struct super_block *sb, uint64_t inode_no);


/**** fin declaracion de funciones ****/

/*
*  Funciones que comienzan por assoofs
*/

	/* 
	* Funcion que obtiene la informacion persistente del inodo del superbloque sb
	*/
	
	struct assoofs_inode_info *assoofs_get_inode_info(struct super_block *sb, uint64_t inode_no){

		//Acceder a disco para leer el bloque que contiene el almacen de inodos
		struct assoofs_inode_info *inode_info = NULL;
		struct buffer_head *bh;

		bh = sb_bread(sb, ASSOOFS_INODESTORE_BLOCK_NUMBER); //usamos sb_bread para leer el superbloque
		inode_info = (struct assoofs_inode_info *)bh->b_data; //estoy leyendo al almacen de inodos asi que tenemos que castear a tipo assoofs_inode_info


		//Recorrer el almacen de inodos en busca del inodo inode no, recibimos como argumento el numero de inodo asi que es lo que vamos a buscar
		struct assoofs_super_block_info *afs_sb = sb->s_fs_info; //lo guardamos en memoria para no acceder a disco tantas veces (en s_fs_info hemos guardado lo qu eleimos antes
		struct assoofs_inode_info *buffer = NULL;
		int i;

		for(i = 0; i < afs_sb->inodes_count; i++) { //recorremos el alamacen de inodos 

			if(inode_info->inode_no == inode_no) {//comparamos el que nos prefuntan que es inode_no con el valor del inodo donde estoy ahora mismo  inode_info->inode_no
				
				buffer = kmalloc(sizeof(struct assoofs_inode_info), GFP_KERNEL); //reservamos memoria diciendole cuanta y donde que siempre usamos esa constante GFP
				memcpy(buffer, inode_info, sizeof(*buffer)); //copia lo de indode info en buffer (es una asignacion para punteros) esta es la copia que se devuelve si se encuentra
				break;
			}
			inode_info++; //sumamos 1 al puntero asi que desplaza el puntero a la siguinete direccion de memmoria
		}


		//Liberal recursos y devolver a informacion del inodo inode no si estaba en el almacen
		brelse(bh);
		printk(KERN_INFO "The inode has been searched\n");

		return buffer;
	}


/*
 *  Operaciones sobre ficheros
 */
ssize_t assoofs_read(struct file * filp, char __user * buf, size_t len, loff_t * ppos);
ssize_t assoofs_write(struct file * filp, const char __user * buf, size_t len, loff_t * ppos);
const struct file_operations assoofs_file_operations = {
    .read = assoofs_read,
    .write = assoofs_write,
};

ssize_t assoofs_read(struct file * filp, char __user * buf, size_t len, loff_t * ppos) {
    printk(KERN_INFO "Read request\n");

    int nbytes=0;
    return nbytes;
}

ssize_t assoofs_write(struct file * filp, const char __user * buf, size_t len, loff_t * ppos) {
    printk(KERN_INFO "Write request\n");
    return len;
}

/*
 *  Operaciones sobre directorios
 */
static int assoofs_iterate(struct file *filp, struct dir_context *ctx);
const struct file_operations assoofs_dir_operations = {
    .owner = THIS_MODULE,
    .iterate = assoofs_iterate,
};

static int assoofs_iterate(struct file *filp, struct dir_context *ctx) {
    printk(KERN_INFO "Iterate request\n");
    return 0;
}

/*
 *  Operaciones sobre inodos
 */
static int assoofs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);
static int assoofs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);
struct dentry *assoofs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags);
static struct inode_operations assoofs_inode_ops = {
    .create = assoofs_create,
    .lookup = assoofs_lookup,
    .mkdir = assoofs_mkdir,
};

struct dentry *assoofs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags) {
    printk(KERN_INFO "Lookup request\n");
    return NULL;
}


static int assoofs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl) {
    printk(KERN_INFO "New file request\n");
    return 0;
}

static int assoofs_mkdir(struct inode *dir , struct dentry *dentry, umode_t mode) {
    printk(KERN_INFO "New directory request\n");
    return 0;
}

/*
 *  Operaciones sobre el superbloque
 */
static const struct super_operations assoofs_sops = {
    .drop_inode = generic_delete_inode,
};

/*
 *  Inicialización del superbloque
 */
int assoofs_fill_super(struct super_block *sb, void *data, int silent) {   
    printk(KERN_INFO "assoofs_fill_super request\n");
    // 1.- Leer la información persistente del superbloque del dispositivo de bloques 
	struct inode *root_inode;
	struct buffer_head *bh;
	struct assoofs_super_block_info *assoofs_sb;
	bh = sb_bread(sb, ASSOOFS_SUPERBLOCK_BLOCK_NUMBER); // sb lo recibe assoofs_fill_super como argumento
	assoofs_sb = (struct assoofs_super_block_info *)bh->b_data;
	 
    // 2.- Comprobar los parámetros del superbloque
	if(assoofs_sb -> magic != ASSOOFS_MAGIC) { //comprobamos el numero magico
		
		printk(KERN_ERR "The magic number is incorrect\n");
		brelse(bh);
		return -1;

	}
	if(assoofs_sb->block_size != ASSOOFS_DEFAULT_BLOCK_SIZE){

		printk(KERN_ERR "The block size is incorrect\n");
		brelse(bh);
		return -1;
	}


    // 3.- Escribir la información persistente leída del dispositivo de bloques en el superbloque sb, incluído el campo s_op con las operaciones que soporta.
	
	sb->s_magic = ASSOOFS_MAGIC; //Asignaremos el número mágico ASSOOFS MAGIC definido en 						assoofs.h al campo s magic del superbloque sb.
	sb->s_maxbytes = ASSOOFS_DEFAULT_BLOCK_SIZE; //Asignaremos el tamaño de bloque ASSOOFS 					DEFAULT BLOCK SIZE definido en assoofs.h al campo s maxbytes del 					superbloque sb.

	sb->s_op = &assoofs_sops; //signaremos operaciones (campo s op al superbloque sb. Las 		operaciones del superbloque se definen como una variable de tipo struct super operations

	sb->s_fs_info = assoofs_sb;
	
    // 4.- Crear el inodo raíz y asignarle operaciones sobre inodos (i_op) y sobre directorios (i_fop)
	extern struct inode *new_inode(struct super_block *sb);

	root_inode = new_inode(sb);

	inode_init_owner(root_inode, NULL, S_IFDIR); // S_IFDIR para directorios, S_IFREG para 		ficheros.

	root_inode->i_ino = ASSOOFS_ROOTDIR_INODE_NUMBER; // número de inodo
	root_inode->i_sb = sb; // puntero al superbloque
	root_inode->i_op = &assoofs_inode_ops; // dirección de una variable de tipo struct (operaciones sobre inodo inode_operations previamente declarada
	root_inode->i_fop = &assoofs_dir_operations; // dirección de una variable de tipo struct 
	root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = current_time(root_inode); // fechas.
	root_inode->i_private = assoofs_get_inode_info(sb, ASSOOFS_ROOTDIR_INODE_NUMBER); //Información persistente del inodo leyendo el bloque de disco y como cargar el inodo se hace varias veces creamos una funcion

	//decirle al struct de entry que le corresponde al dir raiz para cuando monte algo sepa cual es el raiz
	sb->s_root = d_make_root(root_inode);
	if(!sb->s_root) {
		brelse(bh);
		return -1;
	}

	brelse(bh);
    return 0;
}

/*
 *  Montaje de dispositivos assoofs
 */
static struct dentry *assoofs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data) {
    struct dentry *ret = mount_bdev(fs_type, flags, dev_name, data, assoofs_fill_super);
    // Control de errores a partir del valor de ret. En este caso se puede utilizar la macro IS_ERR: if (IS_ERR(ret)) ...
    return ret;
}

/*
 *  assoofs file system type
 */
static struct file_system_type assoofs_type = {
    .owner   = THIS_MODULE,
    .name    = "assoofs",
    .mount   = assoofs_mount,
    .kill_sb = kill_litter_super,
};

static int __init assoofs_init(void) {
    int ret = register_filesystem(&assoofs_type);
    // Control de errores a partir del valor de ret
	if(ret == 0) {

		printk(KERN_INFO "Successfully inited\n");

	}else{

		printk(KERN_ERR "AN error has occurred in the initiation\n");

	}

    return 0;
}

static void __exit assoofs_exit(void) {
    int ret = unregister_filesystem(&assoofs_type);
    // Control de errores a partir del valor de ret

	if(ret == 0) {

		printk(KERN_INFO "Successfully exit\n");

	}else{

		printk(KERN_ERR "AN error has occurred in the exit\n");

	}


}

module_init(assoofs_init);
module_exit(assoofs_exit);

