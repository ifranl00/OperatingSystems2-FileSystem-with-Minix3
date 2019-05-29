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
static struct inode *assoofs_get_inode(struct super_block *sb, int ino);
int assoofs_sb_get_a_freeblock(struct super_block *sb, uint64_t *block);


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
* obtener un puntero al inodo número ino del superbloque sb.
* recibe el sb y el numero de nodo (funcion auxiliar)
*/
static struct inode *assoofs_get_inode(struct super_block *sb, int ino) {

	struct inode *inode;
	assoofs_inode_info *inode_info;
	inode_info = assoofs_get_inode_info(sb, ino); //Obtener la información persistente del inodo ino


	inode = new_inode(sb);//inicializamos el inodo
	
	inode->i_ino = ino; //numero inodo
	inode->i_sb = sb; //superbloque
	inode->i_op = &assoofs_inode_ops; //operaciones inodo
	
	//antes de asignar f_ops dependiendo de si es directorio o archivo
	if (S_ISDIR(inode_info->mode)) {
		inode->i_fop = &assoofs_dir_operations; //operaciones de dir
	}else if (S_ISREG(inode_info->mode)){
		inode->i_fop = &assoofs_file_operations; //operaciones de fichero
	}else{
		printk(KERN_ERR "Unknown inode type. Neither a directory nor a file.");

	}
	
	inode->i_atime = current_time(inode);
	inode->i_mtime = current_time(inode);
	inode->i_ctime = current_time(inode);

	inode->i_private = inode_info; //info persistente al inodo que guardamos previamennte
	
	return inode;
}

/*
*  Obtiene donde hay un bloque libre
*/

int assoofs_sb_get_a_freeblock(struct super_block *sb, uint64_t *block){

	struct assoofs_super_block_info *assoofs_sb = sb->s_fs_info; //
	
	int i;
	for (i = 3; i < ASSOOFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED; i++)
		if (assoofs_sb->free_blocks & (1 << i))
			break; // cuando aparece el primer bit 1 en free_block dejamos de recorrer el mapa de bits, i tiene la posicióndel primer bloque libre
	*block = i; // Escribimos el valor de i en la dirección de memoria indicada como segundo argumento en la función

	assoofs_sb->free_blocks &= ~(1 << i);
	assoofs_save_sb_info(sb);
	/*aqui me quede*/
	return 0;
	

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
	
	struct inode *inode;
	struct super_block *sb;
	assoofs_inode_info *inode_info;
	inode = filp->f_path.dentry->d_inode;
	sb = inode->i_sb;
	inode_info = inode->i_private;
    
	if (ctx->pos) return 0; 
	if ((!S_ISDIR(inode_info->mode))) return -1;

	struct buffer_head *bh;
	bh = sb_bread(sb, inode_info->data_block_number);
	record = (struct assoofs_dir_record_entry *)bh->b_data;

	for (i = 0; i < inode_info->dir_children_count; i++) {
	dir_emit(ctx, record->filename, ASSOOFS_FILENAME_MAXLEN, record->inode_no, DT_UNKNOWN);
	ctx->pos += sizeof(struct assoofs_dir_record_entry);
	record++;
	}

	brelse(bh);
	return 0;
	
}

/*
 *  Operaciones sobre inodos
 */
static int assoofs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);
static int assoofs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);
struct dentry *assoofs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags);
static struct inode_operations assoofs_inode_ops = { //para manejar los inodos
    .create = assoofs_create, //crea nuevos inodos para archivos
    .lookup = assoofs_lookup, 
    .mkdir = assoofs_mkdir,
};


/*
* Busca cualquier inodo de un directorio
* recibe un struct inode el inodo padre, porque para buscar el inodo saber quien es el inodo padre, otro struct dentry que representa la dupla nombre de fichero y numero de inodo y flags que no vamos a usar
*/
struct dentry *assoofs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags) {

	// Accedemos al bloque del disco con el contenido del directorio apuntado por paren_inode
	struct assoofs_inode_info *parent_info = parent_inode->i_private; //me creo una inode info, el campo i private metes la info del inodo que se quiera (i private es de tipo puntero a caracter entonces metes lo que sea) asi ya guardamos ahi la info del inodo padre
	struct super_block *sb = parent_inode->i_sb; //i_sb, hemos guardado el superbloque parar leer el bloque qu econtine  la info del directorio padre
	struct buffer_head *bh;
	bh = sb_bread(sb, parent_info->data_block_number);//con sb_bread le paso ese superbloque y el bloque que tengo que leer saco de parent info que tine el data block number con el numero de bloque que contiene.


	//Recorrer el contenido del directorio buscando la entrada cuyo nombre se corresponda con el que buscamos. Si se localiza la entrada, entonces tenemos construir el inodo correspondiente.
	struct assoofs_dir_record_entry *record; //puntero aux para recorrer el content del directorio (*record)
	record = (struct assoofs_dir_record_entry *)bh->b_data; //accedo al contenido del bloque y hacemos el cast apropiado 
	for (i=0; i < parent_info->dir_children_count; i++) { //for hasta el numero de archivos que continene el directorio padre
		if (!strcmp(record->filename, child_dentry->d_name.name)) { //en cada vuelta compruebo el nombre dell fichero, strcmp devuleve 0 si son iguales
			struct inode *inode = assoofs_get_inode(sb, record->inode_no); // ya tenemos el dir record entry, llamamos a get inode : Función auxiliar que obtine la información de un inodo a partir de su número de inodo.
			inode_init_owner(inode, parent_inode, ((struct assoofs_inode_info *)inode >i_private)->mode); //inode_init_owner(inodo, directorio padre, el modo(un campo de los inodos es el modo (permisoso))
			d_add(child_dentry, inode); //llamo a l add para guardarlo en la herrquia de inodos (excepto el raiz que se crea con otro especial no el d_add)
			return NULL;
		}
		record++;
	}
	
    printk(KERN_INFO "Lookup request\n");
    return NULL;
}

/*
* para crear un archivo es decir crea un inodod que apunte a ese archivo
*/
static int assoofs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl) {
	
	/*----------------crear nuevo inodo----------------------------*/
	struct inode *root_inode;
	struct super_block *sb;
	uint64_t count;

	sb = dir->i_sb; // obtengo un puntero al superbloque desde dir
	count = ((struct assoofs_super_block_info *)sb->s_fs_info)->inodes_count; // obtengo el número de inodos de la información persistente del superbloque

	if(count < ASSOOFS_FYLESYSTEM_OBJECTS_SUPPORTED) {
		inode = new_inode(sb);
		inode->i_ino = (count + ASSOOFS_START_INO - ASSOOFS_RESERVED_INODES + 1); // Asigno número al nuevo inodo a partir de count

		struct assoofs_inode_info *inode_info;
		inode_info = kmalloc(sizeof(struct assoofs_inode_info), GFP_KERNEL);
		inode_info->mode = mode; // El segundo mode me llega como argumento
		inode_info->file_size = 0;
		inode->i_private = inode_info;
		
		inode->i_fop=&assoofs_file_operations; //operaciones ficheros

		assoofs_sb_get_a_freeblock(sb, &inode_info->data_block_number); //para asignarle un bloque libre

		assoofs_add_inode_info(sb, inode_info); //guardar la funcion persistente del nuevo inodo en disco

	/*-------------------modificar el contenido del directorio padre para añadir una nueva entrada-------------------------------------------------------------------------------------------*/
		struct assoofs_inode_info *parent_inode_info;
		struct assoofs_dir_record_entry *dir_contents;
		parent_inode_info = dir->i_private;
		bh = sb_bread(sb, parent_inode_info->data_block_number);
		dir_contents = (struct assoofs_dir_record_entry *)bh->b_data;
		dir_contents += parent_inode_info->dir_children_count; //avanza el puntero el puntero tantos elementos tenga entonces apunta al final
		dir_contents->inode_no = inode_info->inode_no; // inode_info es la información persistente del inodo creado en el paso 2.

		strcpy(dir_contents->filename, dentry->d_name.name); //para asignar cadenas 

		mark_buffer_dirty(bh); //marca como sucio
		sync_dirty_buffer(bh); //sincronizamos
		brelse(bh);

	/*------------------------modificar en el inodo padre para incrementar su numero de hijos----------------------*/
		parent_inode_info->dir_children_count++;
		assoofs_save_inode_info(sb, parent_inode_info); //actualizar esto en disco
		
		
	}else{
		printk(KERN_ERR "New file requested cannot be created\n");
		return -1;
	}


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

