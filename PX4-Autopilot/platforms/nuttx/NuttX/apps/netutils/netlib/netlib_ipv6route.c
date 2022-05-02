/****************************************************************************
 * apps/netutils/netlib/netlib_ipv6route.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <arpa/inet.h>

#include "netutils/netlib.h"

#if defined(CONFIG_NET_IPv6) && defined(HAVE_ROUTE_PROCFS)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Determines the size of an intermediate buffer that must be large enough
 * to handle the longest line generated by this logic.
 */

#define PROCFS_LINELEN 58

/* The form of the entry from the routing table file:
 *
 *            11111111112222222222333333333344444444445555
 *   12345678901234567890123456789012345678901234567890123
 *   nnnn. target:  xxxx:xxxx:xxxx:xxxxxxxx:xxxx:xxxx:xxxx
 *         netmask: xxxx:xxxx:xxxx:xxxxxxxx:xxxx:xxxx:xxxx
 *         router:  xxxx:xxxx:xxxx:xxxxxxxx:xxxx:xxxx:xxxx
 */

#define ADDR_OFFSET  15

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: set_nul_terminator
 *
 * Description:
 *   Make sure that the string is NUL terminated.
 *
 ****************************************************************************/

static void set_nul_terminator(FAR char *str)
{
  /* The first non-hex character that is not ':' terminates the address */

  while ((*str >= '0' && *str <= '9') ||
         (*str >= 'a' && *str <= 'f') ||
         (*str >= 'A' && *str <= 'F') ||
          *str == ':')
    {
       str++;
    }

  *str = '\0';
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: netlib_read_ipv6route
 *
 * Description:
 *   Read the next entry from the IPv6 routing table.
 *
 * Input Parameters:
 *   fd     - The open file descriptor to the procfs' IPv6 routing table.
 *   route  - The location to return that the next routing table entry.
 *
 * Returned Value:
 *   sizeof(struct netlib_ipv6_route_s) is returned on success.  Zero is
 *   returned if the end of file is encountered.  A negated errno value is
 *   returned on any failure.
 *
 ****************************************************************************/

ssize_t netlib_read_ipv6route(FILE *stream,
                              FAR struct netlib_ipv6_route_s *route)
{
  char line[PROCFS_LINELEN];
  FAR char *addr;
  int ret;

  DEBUGASSERT(stream != NULL && route != NULL);

  /* The form of the entries read from the routing table:
   *
   *            1111111111222222222233333333334444444444555
   *   1234567890123456789012345678901234567890123456789012
   *   nnnn. target:  xxxx:xxxx:xxxx:xxxxxxxx:xxxx:xxxx:xxxx
   *         netmask: xxxx:xxxx:xxxx:xxxxxxxx:xxxx:xxxx:xxxx
   *         router:  xxxx:xxxx:xxxx:xxxxxxxx:xxxx:xxxx:xxxx
   */

  /* Read the first line from the file */

  if (fgets(line, PROCFS_LINELEN, stream) == NULL)
    {
      /* End of file (or possibly a read error?) */

      return 0;
    }

  /* The first line of the group should consist of a number index */

  if (line[0] < '0' || line[0] > 9)
    {
      return -EINVAL;
    }

  /* Make certain that there is a NUL terminator */

  line[PROCFS_LINELEN - 1] = '\0';

  /* Convert the prefix address to binary */

  addr = &line[ADDR_OFFSET];
  set_nul_terminator(addr);

  ret = inet_pton(AF_INET6, addr, &route->prefix);
  if (ret < 0)
    {
      return ret;
    }
  else if (ret == 0)
    {
      return -EINVAL;
    }

  /* Read the second line from the file */

  if (fgets(line, PROCFS_LINELEN, stream) == NULL)
    {
      /* End of file (or possibly a read error?) */

      return 0;
    }

  /* The second line of the group should not include an index */

  if (line[0] != ' ')
    {
      return -EINVAL;
    }

  /* Make certain that there is a NUL terminator */

  line[PROCFS_LINELEN - 1] = '\0';

  /* Convert the prefix address to binary */

  addr = &line[ADDR_OFFSET];
  set_nul_terminator(addr);

  ret = inet_pton(AF_INET6, addr, &route->netmask);
  if (ret < 0)
    {
      return ret;
    }
  else if (ret == 0)
    {
      return -EINVAL;
    }

  /* Read the third line from the file */

  if (fgets(line, PROCFS_LINELEN, stream) == NULL)
    {
      /* End of file (or possibly a read error?) */

      return 0;
    }

  /* The third line of the group should not include an index */

  if (line[0] != ' ')
    {
      return -EINVAL;
    }

  /* Make certain that there is a NUL terminator */

  line[PROCFS_LINELEN - 1] = '\0';

  /* Convert the prefix address to binary */

  addr = &line[ADDR_OFFSET];
  set_nul_terminator(addr);

  ret = inet_pton(AF_INET6, addr, &route->router);
  if (ret < 0)
    {
      return ret;
    }
  else if (ret == 0)
    {
      return -EINVAL;
    }

  return sizeof(struct netlib_ipv6_route_s);
}

#endif /* CONFIG_NET_IPv6 && HAVE_ROUTE_PROCFS */
