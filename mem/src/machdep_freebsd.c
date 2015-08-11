#include <e.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>

#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/user.h>

#include "e_mod_main.h"

#define GETSYSCTL(name, var)    getsysctl(name, &(var), sizeof (var))

static int
getsysctl (char *name, void *ptr, size_t len)
{
  size_t nlen = len;
  if (sysctlbyname (name, ptr, &nlen, NULL, 0) == -1)
    {
      return (1);
    }

  if (nlen != len)
    {
      return (1);
    }

  return (0);
}

static int
swapinfo (int *total, int *used)
{
  int pagesize = getpagesize ();
  size_t mibsize, size;
  struct xswdev xsw;
  int mib[16], n;
  int tmp_total, tmp_used;

  *total = 0;
  *used = 0;

  mibsize = sizeof mib / sizeof mib[0];
  if (sysctlnametomib ("vm.swap_info", mib, &mibsize) == -1)
    {
      warn ("sysctlnametomib()");
      return 1;
    }

  for (n = 0;; n++)
    {
      mib[mibsize] = n;
      size = sizeof xsw;
      if (sysctl (mib, mibsize + 1, &xsw, &size, NULL, 0) == -1)
	break;

      if (xsw.xsw_version != XSWDEV_VERSION)
	{
	  warnx ("xswdev version mismatch");
	  return 1;
	}

      tmp_total = (long long) xsw.xsw_nblks * pagesize;
      tmp_used = (long long) xsw.xsw_used * pagesize;
      *total += tmp_total;
      *used += tmp_used;
    }
  if (errno != ENOENT)
    warn ("sysctl()");

  return 0;
}

void
_mem_get_values (ci, phys_used, sw_used, phys_total, sw_total)
     Config_Item *ci;
     int *phys_used;
     int *sw_used;
     int *phys_total;
     int *sw_total;
{
  int total_pages, inactive_pages, free_pages;

  int pagesize = getpagesize ();

  if (GETSYSCTL ("vm.stats.vm.v_page_count", total_pages))
    {
      warnx ("can't read sysctl \"vm.stats.vm.v_page_count\"");
      return;
    }

  if (GETSYSCTL ("vm.stats.vm.v_free_count", free_pages))
    {
      warnx ("can't read sysctl \"vm.stats.vm.v_free_count\"");
      return;
    }

  if (GETSYSCTL ("vm.stats.vm.v_inactive_count", inactive_pages))
    {
      warnx ("can't read sysctl \"vm.stats.vm.v_inactive_count\"");
      return;
    }

  *phys_total = (total_pages * pagesize) >> 10;
  *phys_used = ((total_pages - free_pages - inactive_pages) * pagesize) >> 10;

  if ((swapinfo (sw_total, sw_used)) != 0)
    {
      *sw_total = 0;
      *sw_used = 0;
    }
}
