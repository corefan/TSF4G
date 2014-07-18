#include "appender/tlog_appender_rolling_file.h"
#include <string.h>
#include <sys/stat.h>

void tlog_appender_rolling_file_init(tlog_appener_rolling_file_t *self, const tlog_config_appender_rolling_file_t *config)
{
	char file_name[TSERVER_FILE_NAME_LENGH];
	char *ch;

	strncpy(file_name, config->file_name, TSERVER_FILE_NAME_LENGH);
	file_name[TSERVER_FILE_NAME_LENGH - 1] = 0;
	for(ch = file_name; ch; ++ch)
	{
		if(*ch == '/')
		{
			*ch = 0;
			mkdir(file_name, 0755);
			*ch = '/';
		}
	}	

	self->fout = NULL;
	self->index = 0;
}

void tlog_appender_rolling_file_log(tlog_appener_rolling_file_t *self, const tlog_config_appender_rolling_file_t *config, const tlog_message_t *message)
{
	size_t file_size;
	long ft;
	size_t msg_len;

    
	if(self->fout == NULL)
	{
		self->fout = fopen(config->file_name, "wb+");
		if(self->fout == NULL)
		{
			goto done;
		}
		fseek(self->fout, 0, SEEK_END);		
	}
	
	ft = ftell(self->fout);
	if(ft < 0)
	{
	    goto done;
	}
	else
	{
	    file_size = (size_t)ft;
	}

	msg_len = strlen(message->msg);
	if(file_size + msg_len > config->max_file_size)
	{
		char file_name[TSERVER_FILE_NAME_LENGH];
		snprintf(file_name, TSERVER_FILE_NAME_LENGH, "%s.%u", config->file_name, self->index);
		++self->index;
		if(self->index > config->max_backup_index)
		{
			self->index = 0;
		}
		
		fclose(self->fout);
		self->fout = NULL;
		
		rename(config->file_name, file_name);
		
		self->fout = fopen(config->file_name, "wb+");
		if(self->fout == NULL)
		{
			goto done;
		}
		fseek(self->fout, 0, SEEK_END);		
	}
	
	fwrite(message->msg, 1, msg_len, self->fout);
	fputc('\n', self->fout);
	fflush(self->fout);
	
done:
	return;
}

void tlog_appender_rolling_file_fini(tlog_appener_rolling_file_t *self)
{
	if(self->fout != NULL)
	{
	    fclose(self->fout);
        self->fout = NULL;
        self->index = 0;
	}
}

