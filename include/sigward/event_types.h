// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/* Copyright (c) Sigward-Authors */

#ifndef __SIGWARD_EVENT_TYPES_H
#define __SIGWARD_EVENT_TYPES_H

enum sigward_event_type {
  EVENT_TYPE_UNKNOWN = 0,

  // Binaries (lsm/bprm_check_security)
  EVENT_TYPE_UNATTESTED_BINARY = 1,
  EVENT_TYPE_HASH_MISMATCH = 2,

  // Shared objects (lsm/mmap_file, PROT_EXEC)
  EVENT_TYPE_UNATTESTED_SHARED_OBJECT = 3,
  EVENT_TYPE_SHARED_OBJECT_HASH_MISMATCH = 4,
};

#endif /* __SIGWARD_EVENT_TYPES_H */
