#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <cutils/ashmem.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/ashmem.h>

class Ashmem {
  public:
    Ashmem(size_t size, const char *name)
      : _size(size)
    {
      _name = strdup(name);
      _fd = ashmem_create_region(name, size);
      int err = ashmem_set_prot_region(_fd, PROT_READ | PROT_WRITE);
      if (err) {
        printf("------ ashmem_set_prot_region(%d) failed %d %d\n",
               _fd, err, errno);
      }
      _addr = mmap(NULL, _size, PROT_READ | PROT_WRITE, MAP_PRIVATE, _fd, 0);
      if (_addr == MAP_FAILED) {
        printf("---------- mmap failed for imageref_ashmem size=%d err=%d\n",
               _size, errno);
      }
      
    };
    void report() {
      int pin_status;
      char name_buf[ASHMEM_NAME_LEN];
      struct ashmem_pin pin = { 0, 0 };
      pin_status =  ioctl(_fd, ASHMEM_GET_PIN_STATUS, &pin);
      size_t qsize = ashmem_get_size_region(_fd);
      ioctl (_fd, ASHMEM_GET_NAME, name_buf);
      printf("Reporting ashmem:\n");
      printf("  size %zu, %zu\n", qsize, _size);
      printf("  name %s, %s\n", _name, name_buf);
      printf("  pin %d, %d\n",
             pin_status,
             _pin);
      printf("  address %p\n", _addr);
    };
    bool isPinned() {
      return _pin;  
    };
    void pin() {
      ashmem_pin_region(_fd, 0, 0);
      _pin = true;
    }
    void unpin() {
      ashmem_unpin_region(_fd, 0, 0);
      _pin = false;
    }
  private:
    size_t _size;
    const char *_name;
    int _fd;
    void *_addr;
    bool _pin;
};

Ashmem ashmem1(1024, "test1");
Ashmem ashmem2(1024*4, "test2");

int main(int argc, char* argv[])
{
  int count = 1;
  ashmem1.pin();
  ashmem2.pin();
  ashmem2.unpin();
  while (1)
  {
      ashmem1.report();
      ashmem2.report();
	  fflush(stdout);
	  sleep(2);
  }
  return 0;
}
