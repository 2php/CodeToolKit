#include <mmsx.h>
#include <stdio.h>
#include <string.h>
#include <string>
using namespace std;

#define __MAIN__

class downloader
{
public:
	downloader()
	{
		mms = NULL;
		buffer = new char[1024];
		fp = fopen("downloaded.mms", "wb");
	}

	~downloader()
	{
		close();

		if( fp )
		{
			fclose(fp);
			fp = NULL;
		}
		if( buffer )
		{
			delete buffer;
			buffer = NULL;
		}
	}

	int length()
	{
		return mmsx_get_length(mms);
	}

	int position()
	{
		return mmsx_get_current_pos(mms);
	}

	int duration()
	{
		return mmsx_get_time_length(mms);
	}

	bool seekable()
	{
		return mmsx_get_seekable(mms);
	}

	int seek(int pos)
	{
		return mmsx_seek(NULL, mms, pos, 0);
	}

	int time_seek(long time)
	{
		return mmsx_time_seek(NULL, mms, float(time));
	}

	int read()
	{
		memset(buffer, 0, 1024);
		count = mmsx_read(NULL, mms, buffer, 1024);
		if ( count<0 )
		{
			printf("read data from mms url error!\n");
		}
		return (int)buffer[count];
	}

	void download()
	{
		while ( 1 )
		{
			int num = read();
			if ( num>0 )
			{
				printf("read data ok! %d\n", num);
				//Ð´ÈëÎÄ¼þ
				fwrite(buffer, count, 1, fp);
			}
			else
			{
				printf("no more data!!!\n");
				break;
			}			
		}
	}

	bool open(const char* url, long bandwidth)
	{
		if( mms )
		{
			mmsx_close(mms);
			mms = NULL;
		}
		mms = mmsx_connect(NULL, NULL, url, int(bandwidth));
		if( !mms )
		{
			printf("connect to mms url error!\n");
			return false;
		}

		return true;
	}

	void close()
	{
		if( mms )
		{
			mmsx_close(mms);
			mms = NULL;
		}
	}

private:
	mmsx_t* mms;
	char* buffer;
	long count;
	FILE* fp;
};


#ifdef __MAIN__
int main(int argc, char* argv[])
{
	downloader mmskeep;
	mmskeep.open("mms://winmedia.cctv.com.cn/baijiajiangtan/2007/12/baijiajiangtan_300_20071220_1.wmv", 100000);
	if( mmskeep.length()>0 )
	{
		mmskeep.time_seek(2300);
		mmskeep.download();
		mmskeep.close();
	}
}
#endif
