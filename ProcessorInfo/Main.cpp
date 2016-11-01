
#include <windows.h>
#include <stdio.h>


int main()
{
	DWORD buffer_size = 0;
	GetLogicalProcessorInformation(0, &buffer_size);
	DWORD nb_buffers = buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

	SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = new SYSTEM_LOGICAL_PROCESSOR_INFORMATION[nb_buffers];
	GetLogicalProcessorInformation(buffer, &buffer_size);

	for (DWORD i = 0; i < nb_buffers; i++)
	{
		if (buffer[i].Relationship == RelationCache)
		{
			printf("Processor (%2d) L%d: size %d bytes, line size %d bytes\n",
				buffer[i].ProcessorMask,
				buffer[i].Cache.Level,
				buffer[i].Cache.Size,
				buffer[i].Cache.LineSize);
			//break;
		}
	}

	delete buffer;
	return 0;
}