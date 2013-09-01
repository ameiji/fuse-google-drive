/*
	fuse-google-drive: a fuse filesystem wrapper for Google Drive
	Copyright (C) 2012  James Cline

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
 	published by the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#define FUSE_USE_VERSION 26
#include <dirent.h>
#include <errno.h>
#include <fuse.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
//#include <iconv.h>

#include "gd_cache.h"
#include "gd_interface.h"
#include "str.h"

/**
 *  Store any state about this mount in this structure.
 *
 *  root: The path to the mount point of the drive.
 *  credentials: Struct which stores necessary credentials for this mount.
 */
struct gd_state {
	char* root;
	struct gdi_state gdi_data;
};

/** Get file attributes.
 *
 */
int gd_getattr (const char *path, struct stat *statbuf)
{
	struct fuse_context *fc = fuse_get_context();
	char pathenc[MAXDIRNAME];
	struct gd_fs_entry_t * entry;
	const char *filename;
	
	if (strlen(encoding)>0 && decode(path, pathenc, sizeof(pathenc), encoding)!=-1)
	    path = pathenc;

	if( strcmp("/", path) == 0)
	{
		statbuf->st_mode = S_IFDIR | 0700;
		statbuf->st_nlink=2;
	}
	else
	{
		memset(statbuf, 0, sizeof(struct stat));
		filename = gdi_strip_path(path);
		entry = gd_fs_entry_find(filename);

		if(!entry)
			return -ENOENT;

		if(entry)
			statbuf->st_size = entry->size;

		statbuf->st_mode = entry->mode;
		statbuf->st_nlink=1;
	}
	statbuf->st_uid = fc->uid;
	statbuf->st_gid = fc->gid;

	return 0;
}

/** Read the target of a symbolic link.
 *
 */
int gd_readlink (const char *path, char *link, size_t size)
{
	return 0;
}

/** Create a regular file.
 *
 */
int gd_mknod (const char *path, mode_t mode, dev_t dev)
{
	return 0;
}

/** Create a directory.
 *
 */
int gd_mkdir (const char *path, mode_t mode)
{
	struct gd_fs_tableinode_t *binode;
	for(binode = busy_inodes ; binode->next!=NULL; binode=binode->next)
	{
		if (binode->num < 2)
			continue;
		fprintf(stderr, "[TEST_INODE_CREATE] Inode[%ld] Filename[%s] parent[%ld]\n", binode->num, binode->inode->node->filename.str, binode->pnum);
	}
	return 0;
}

/** Remove a file.
 *
 */
int gd_unlink (const char *path)
{
	return 0;
}

/** Remove a directory.
 *
 */
int gd_rmdir (const char *path)
{
	return 0;
}

/** Create a symbolic link.
 *
 *  Google Drive likely does not support an equivalent operation.
 *  We could allow this for a specific session, but it would be lost
 *  after an unmount.
 */
int gd_symlink (const char *path, const char *link)
{
	return 0;
}

/** Rename a file.
 *
 */
int gd_rename (const char *path, const char *newpath)
{
	return 0;
}

/** Create a hard link to a file.
 *
 *  Google Drive likely does not support an equivalent operation.
 *  We could allow this for a specific session, but it would be lost
 *  after an unmount.
 */
int gd_link (const char *path, const char *newpath)
{
	return 0;
}

/** Change the permission bits of a file.
 *
 *  Could this be used to share a document with others?
 *  Perhaps o+r would make visible to all, o+rw would be editable by all?
 *  I can't think of a way to do any sort of group level permissions sanely atm.
 *
 *  Alternatively, these permission settings could be used for local acces only,
 *  which would let you make a file locally readable or modifiable by another
 *  system user. This may violate the principle of least astonishment least.
 */
int gd_chmod (const char *path, mode_t mode)
{
	return 0;
}

/** Change the owner and group of a file.
 *
 *  Since this uses gid and uids, I cannot think of a way to easily use this for
 *  Google Drive sharing purposes with specific users right now.
 *  Perhaps some additional utility could be used to display a mapping of
 *  uid/gids and Google Drive users you can share with?
 *
 *  Alternatively, these uid/gid settings could be used for local acces only,
 *  which would let you make a file locally readable or modifiable by another
 *  system user. This may violate the principle of least astonishment least.
 */
int gd_chown (const char *path, uid_t uid, gid_t gid)
{
	return 0;
}

/** Change the size of a file.
 *
 */
int gd_truncate (const char *path, off_t newsize)
{
	return 0;
}

/** File open operation.
 *
 */
int gd_open (const char *path, struct fuse_file_info * fileinfo)
{
	struct gdi_state *state = &((struct gd_state*)fuse_get_context()->private_data)->gdi_data;

	int flags = fileinfo->flags;
	/*
	  Is it possible to have a file in drive or docs you cannot read?
		I suppose it may be the case that you lost read access since last checked,
		but between open() and read() that can happen anyway, so why check here at
		all?
		Maybe something in the Access Control Lists allows this?
	*/
	if(flags & O_RDONLY)
	{

	}

	// TODO: everything below here
	// NOTE: O_RDWR won't work, just O_RDONLY
	// people can share read only files with you
	if(flags & O_WRONLY)
	{

	}

	if(flags & O_RDWR)
	{

	}

	// Don't need to check O_CREAT, O_EXCL, O_TRUNC
	// Do we need to check all these?
	/* Comment these out for now to reduce user headacke
	if(flags & O_APPEND);
	if(flags & O_ASYNC);
	//if(flags & O_DIRECT);
	// if(flags & O_DIRECTORY); // opendir() only?
	// if(flags & O_LARGEFILE);
	//if(flags & O_NOATIME); // does google drive do this anyway?
	if(flags & O_NOCTTY); // does this do anything/is it passed to us at all?
	if(flags & O_NOFOLLOW);
	if(flags & O_NONBLOCK || flags & O_NONBLOCK); // read man 2 fcntl and man 7 fifo
	if(flags & O_SYNC);
	*/

	// If we have access to this file, then load it. 
	// TODO: Make gdi_load() nonblocking if appropriate
	const char* filename = gdi_strip_path(path);
	struct gd_fs_entry_t *entry = gd_fs_entry_find(filename);
	int load = gdi_load(state, entry);
	if(load)
		return -1;

	return 0;
}

/** Read data from an open file.
 *
 */
int gd_read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileinfo)
{
	struct gdi_state *state = &((struct gd_state*)fuse_get_context()->private_data)->gdi_data;
	const char* filename = gdi_strip_path(path);
	struct gd_fs_entry_t *entry = gd_fs_entry_find(filename);
	if(!entry)
		return 0;
	size_t length = size;
	const char const* chunk = gdi_read(&length, entry, offset);
	memcpy(buf, chunk, length);
	return length;
}

/** Write data to an open file.
 *
 */
int gd_write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Get file system statistics.
 *
 */
int gd_statfs (const char *path, struct statvfs *statv)
{
	return 0;
}

/** Possibly flush cached data.
 *
 */
int gd_flush (const char *path, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Release an open file.
 *
 */
int gd_release (const char *path, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Synchronize file contents.
 *
 */
int gd_fsync (const char *path, int datasync, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Set extended attributes.
 *
 *  Does this mean anything for Google Drive?
 */
int gd_setxattr (const char *path, const char *name, const char *value, size_t size, int flags)
{
	return 0;
}

/** Get extended attributes.
 *
 *  Does this mean anything for Google Drive?
 */
int gd_getxattr (const char *path, const char *name, char *value, size_t size)
{
	return 0;
}

/** List extended attributes.
 *
 *  Does this mean anything for Google Drive?
 */
int gd_listxattr (const char *path, char *list, size_t size)
{
	return 0;
}

/** Remove extended attributes.
 *
 *  Does this mean anything for Google Drive?
 */
int gd_removexattr (const char *path, const char *name)
{
	return 0;
}

/** Open a directory.
 *
 */
int gd_opendir (const char *path, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Read directory.
 *
 */
int gd_readdir (const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileinfo)
{
	char outBuffer[MAXDIRNAME];
	size_t iconverr;
	unsigned long inode;
	const char *filename;
	struct gd_fs_entry_t * entry;

	fprintf(stderr, "readdir(): %s\n", path);

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	struct gdi_state *state = &((struct gd_state*)fuse_get_context()->private_data)->gdi_data;

	struct gd_fs_entry_t *iter = state->head;
	if (!strcmp(path, "/"))
	{
	    while(iter != NULL)
	    {
		inode = iter->inode;
		if (inodetable[inode].pnum != 1)
		{
		    iter = iter->next;
		continue;
		}
		if (strlen(encoding)>0 && encode(iter->filename.str, outBuffer, sizeof(outBuffer), encoding)!=-1)
		{
		    if(filler(buf, outBuffer, NULL, 0))
		    {
			fprintf(stderr, "readdir() filler()\n");
			return -ENOMEM;
		    }
		    iter = iter->next;
		}
		else
		{
		    if(filler(buf, iter->filename.str, NULL, 0))
		    {
			fprintf(stderr, "readdir() filler()\n");
			return -ENOMEM;
		    }
		    iter = iter->next;
		}
	    }
	}
	else
	{
		filename = gdi_strip_path(path);
		entry = gd_fs_entry_find(filename);
	    while(iter != NULL)
	    {
		inode = iter->inode;
		if (inodetable[inode].pnum != entry->inode)
		{
		    iter = iter->next;
		continue;
		}
		if (strlen(encoding)>0 && encode(iter->filename.str, outBuffer, sizeof(outBuffer), encoding)!=-1)
		{
		    if(filler(buf, outBuffer, NULL, 0))
		    {
			fprintf(stderr, "readdir() filler()\n");
			return -ENOMEM;
		    }
		    iter = iter->next;
		}
		else
		{
		    if(filler(buf, iter->filename.str, NULL, 0))
		    {
			fprintf(stderr, "readdir() filler()\n");
			return -ENOMEM;
		    }
		    iter = iter->next;
		}
	    }
	}

	return 0;
}

/** Release directory.
 *
 */
int gd_releasedir (const char *path, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Synchronize directory contents.
 *
 */
int gd_fsyncdir (const char *path, int datasync, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Initialize filesystem
 *
 */
void *gd_init (struct fuse_conn_info *conn)
{
	return ((struct gd_state *) fuse_get_context()->private_data);
}

/** Clean up filesystem
 *
 *  Automatically called by fuse on filesystem exit (unmount).
 */
void gd_destroy (void *userdata)
{
}

/** Check file access permission.
 *
 */
int gd_access (const char *path, int mask)
{
	return 0;
}

/** Create and open a file.
 *
 */
int gd_create (const char *path, mode_t mode, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Change the size of an open file.
 *
 */
int gd_ftruncate (const char *path, off_t offset, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Get attributes from an open file.
 *
 */
int gd_fgetattr (const char *path, struct stat *statbuf, struct fuse_file_info *fileinfo)
{
	return 0;
}

/** Perform POSIX file locking operation.
 *
 *  Does this make any sense for Google Drive?
 */
int gd_lock (const char *path, struct fuse_file_info *fileinfo, int cmd, struct flock *lock)
{
	return 0;
}

/** Change the access and modification times of a file with nanosecond
 *  resoluton.
 *
 *  Does this make any sense for Google Drive?
 */
int gd_utimens (const char *path, const struct timespec tv[2])
{
	return 0;
}

/** Ioctl
 *
 *  Does this make any sense for Google Drive?
 */
int gd_ioctl (const char *path, int cmd, void *arg, struct fuse_file_info *fileinfo, unsigned int flags, void *data)
{
	return 0;
}

/** Poll for IO readiness events.
 *
 */
int gd_poll (const char *path, struct fuse_file_info *fileinfo, struct fuse_pollhandle *ph, unsigned *reventsp)
{
	return 0;
}


// Only uncomment these assignments once an operation's function has been
// fleshed out.
struct fuse_operations gd_oper = {
	.getattr     = gd_getattr,
	//.readlink    = gd_readlink,
	// getdir() deprecated, use readdir()
	.getdir        = NULL,
	//.mknod       = gd_mknod,
	.mkdir       = gd_mkdir,
	//.unlink      = gd_unlink,
	//.rmdir       = gd_rmdir,
	//.symlink     = gd_symlink,
	//.rename      = gd_rename,
	//.link        = gd_link,
	//.chmod       = gd_chmod,
	//.chown       = gd_chown,
	//.truncate    = gd_truncate,
	// utime() deprecated, use utimens
	.utime         = NULL,
	.open        = gd_open,
	.read        = gd_read,
	//.write       = gd_write,
	//.statfs      = gd_statfs,
	//.flush       = gd_flush,
	.release     = gd_release,
	//.fsync       = gd_fsync,
	//.setxattr    = gd_setxattr,
	//.getxattr    = gd_getxattr,
	//.listxattr   = gd_listxattr,
	//.removexattr = gd_removexattr,
	//.opendir     = gd_opendir,
	.readdir     = gd_readdir,
	//.releasedir  = gd_releasedir,
	//.fsyncdir    = gd_fsyncdir,
	.init        = gd_init,
	//.destroy     = gd_destroy,
	//.access      = gd_access,
	//.create      = gd_create,
	//.ftruncate   = gd_ftruncate,
	//.getattr     = gd_fgetattr,
	//.lock        = gd_lock,
	//.utimens     = gd_utimens,
	//.ioctl       = gd_ioctl,
	//.poll        = gd_poll,
};

void usage(char * pname) 
{
	printf("Usage: %s [-C Encoding] mountpoint\n", pname);
	exit(0);
}

static int gdi_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs)
{
	switch (key) {
		case KEY_HELP:
			fprintf(stderr,
					"usage: %s mountpoint [options]\n"
					"\n"
					"general options:\n"
					"    -o opt,[opt...]  mount options\n"
					"    -h   --help      print help\n"
					"    -V   --version   print version\n"
					"\n"
					"Myfs options:\n"
					"    -o mynum=NUM\n"
					"    -o mystring=STRING\n"
					"    -o mybool\n"
					"    -o nomybool\n"
					"    -n NUM           same as '-omynum=NUM'\n"
					"    --mybool=BOOL    same as 'mybool' or 'nomybool'\n"
					, outargs->argv[0]);
			fuse_opt_add_arg(outargs, "-ho");
			fuse_main(outargs->argc, outargs->argv, &gd_oper, NULL);
			exit(1);

		case KEY_VERSION:
			fprintf(stderr, "Myfs version %s\n", PACKAGE_VERSION);
			fuse_opt_add_arg(outargs, "--version");
			fuse_main(outargs->argc, outargs->argv, &gd_oper, NULL);
			exit(0);
	}
	return 1;
}

int main(int argc, char* argv[])
{
	int fuse_stat,
	    ret;
	struct gd_state gd_data;
	int opt=0, optc=0;
	char keyvalue[MAXENCODELEN];
	unsigned long finodes, binodes;

	ret = init_inode_table();
	if (ret)
	{
	    fprintf(stderr, "Init table of inodes filed\n");
	    return ret;
	}

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct gdi_config conf;

	memset(&conf, 0, sizeof(conf));
	fuse_opt_parse(&args, &conf, gdi_opts, gdi_opt_proc);

	unsigned long inode = get_free_inode();
	if (inode)
	{
	    fprintf(stderr, "Get inode %ld is OK. Busy inodes: %d, Free Inodes: %d\n", inode, istat.busy, istat.free);
	}
	else 
	{
	    fprintf(stderr, "Get test inode FAILED. Busy inodes: %d, Free Inodes: %d\n", istat.busy, istat.free);
	    return 1;
	}
	if ((ret = free_inode(inode))!=0)
	{
	    fprintf(stderr, "Free inode FAILED. Busy inodes: %d, Free Inodes: %d\n", istat.busy, istat.free);
	    return 1;
	}
	else
	    fprintf(stderr, "Free inode %ld is OK. Busy inodes: %d, Free Inodes: %d\n", inode, istat.busy, istat.free);

	fprintf(stderr, "Test inode[%ld]: state[%d] address[%p]\n", 0, inodetable[0].state, &inodetable[0]);
	if (conf.fulltest)
	{
	    finodes = get_all_free_inodes();
	    binodes = get_all_busy_inodes();
	    fprintf(stderr, "Test inode[%ld]: state[%d] address[%p]\n", 0, inodetable[0].state, &inodetable[0]);
	    fprintf(stderr, "Free inodes [%ld] Busy inodes [%ld]\n", finodes, binodes);
	    if ((finodes+binodes)!=istat.allocated || finodes != istat.free || binodes != istat.busy) /* 0 inode = 1 inode */
	    {
		fprintf(stderr, "Inode's table[%p] is corrupted: Allocated[%ld %ld] Free[%ld %ld] Busy[%ld %ld]\n", inodetable, finodes+binodes, istat.allocated, finodes, istat.free, binodes, istat.busy);
		return 1;
	    }
	};

	ret = gdi_init(&gd_data.gdi_data);
	if(ret != 0)
		return ret;

	// Start fuse
	if (strlen(conf.encoding)>0)
	{
	    fprintf(stderr, "Encoding: from UTF-8 to %s\n", conf.encoding);
	    encoding = conf.encoding;
	}
	else
	    encoding = NULL;

	fuse_stat = fuse_main(args.argc, args.argv, &gd_oper, &gd_data);
	/*  When we get here, fuse has finished.
	 *  Do any necessary cleanups.
	 */
	gdi_destroy(&gd_data.gdi_data);

	return fuse_stat;
}
