#include "mmap.h"
void *open_mmap(char *file_name){
	void *addr;
	int fd;
	struct stat sb;
	off_t offset, pa_offset;
	size_t length;
	fd = open(file_name, O_RDWR);
	if (fd == -1)
		handle_error("open");
	if (fstat(fd, &sb) == -1) /* To obtain file size */
		handle_error("fstat");
	offset = 0;
	pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);	/* offset for mmap() must be page aligned */
	if (offset >= sb.st_size) {
		fprintf(stderr, "offset is past end of file\n");
		exit(EXIT_FAILURE);
	}
    length = sb.st_size - offset;
	addr = mmap(NULL, length + offset - pa_offset, PROT_READ|PROT_WRITE, MAP_SHARED, fd, pa_offset);
	if(addr == MAP_FAILED)
		handle_error("mmap");
	close(fd);
	return addr;
}
void close_mmap(void *addr, char *file_name){
	int fd;
	struct stat sb;
	off_t offset, pa_offset;
	size_t length;
	fd = open(file_name, O_RDWR);
	if (fd == -1)
		handle_error("open");
	if (fstat(fd, &sb) == -1) /* To obtain file size */
		handle_error("fstat");
	length = sb.st_size;
	close(fd);
	munmap(addr , length);
}
