#include "copyfile.h"


int check_type(const char *path)
{
	struct stat info;
	stat(path,&info);
	if(S_ISREG(info.st_mode))
	{
		return 0;
	}
	if(S_ISDIR(info.st_mode))
	{
		return 1;
	}

	return -1;
}

void copy_dir(struct thread_pool *pool,const char *src_dir,const char *dst_dir)
{
	//Create the new dir in new path 
	int ret;
	
	if(!opendir(dst_dir))
	{
		ret = mkdir(dst_dir,0777);
		if(ret == -1)
		{
			printf("The new dir create error!\n");
			exit(1);
		}
	}
	
	printf("The [%u]pthread is copying dirent: %s\n",(unsigned)pthread_self(),dst_dir);
	
	//check the child-dir in the old dir
	pthread_mutex_lock(&pool->lock);
	pool->dirent_count++;
	pthread_mutex_unlock(&pool->lock);
	
	DIR *o_dir = opendir(src_dir);
	
	struct dirent *dp = NULL;
	while((dp = readdir(o_dir))!=NULL)
	{
		if(dp->d_name[0]=='.')
			continue;
		
		//save the new dir path
		char dst_path[100];
		bzero(dst_path,sizeof(dst_path));
		strcat(dst_path,dst_dir);
		//Ensure that the connection symbol is correct
		if(dst_dir[strlen(dst_dir)-1]!='/')
			strcat(dst_path,"/");
		
		strcat(dst_path,dp->d_name);
		//save the old dir path
		char src_path[100];
		bzero(src_path,sizeof(src_path));
		strcat(src_path,src_dir);
		//Ensure that the connection symbol is correct
		if(src_dir[strlen(src_dir)-1]!='/')
			strcat(src_path,"/");

		strcat(src_path,dp->d_name);
		
		//copy
		FILES *name = malloc(sizeof(FILES));
		strcpy(name->src_file, src_path);
		strcpy(name->dst_file, dst_path);
		
		
		if(dp->d_type == DT_DIR)
			copy_dir(pool,src_path,dst_path);
		else
			add_task(pool,copy,name);
		
		
	}
	
	closedir(o_dir);
}

void copy_file(const char *src_file,const char *dst_file)
{	
	
	printf("The [%u]pthread is copying file: %s\n",(unsigned)pthread_self(),src_file);
	//OPEN FILE
	FILE *s_file = fopen(src_file,"r");
	
	if(s_file == NULL)
	{
		printf("Can not find file !\n");
		exit(1);
	}
	FILE *t_file = fopen(dst_file,"w");
	if(t_file == NULL)
	{
		printf("Create target file error!\n");
		exit(1);
	}
	//COPY FILE
	char data_buf[500];
	int n;
	long data_hd,data_tl;
	while(1)
	{
		data_hd = ftell(s_file);
		n = fread(data_buf,50,5,s_file);
		if(n==5)
		{
			fwrite(data_buf,50,1,t_file);
		}
		if(n<5)
		{
			data_tl=ftell(s_file);
			fwrite(data_buf,data_tl-data_hd,1,t_file);
			break;
		}
		
	}
	
	
	fclose(t_file);
	fclose(s_file);
}