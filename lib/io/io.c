/* SPDX-License-Identifier: GLP-2.0-only */

#include "io.h"

#include "connection/init.h"
#include "system/init.h"

void IO_object_init(struct io_object_s *object, unsigned char type)
{

    memset(object, 0, sizeof(struct io_object_s));

    switch (type) {

	case IO_OBJECT_TYPE_CONNECTION:

	    object->type=type;
	    IO_connection_object_init(&object->io.connection);
	    object->backend=&object->io.connection.backend;
	    break;

	case IO_OBJECT_TYPE_SYSTEM:

	    object->type=type;
	    IO_system_object_init(&object->io.system);
	    object->backend=&object->io.system.backend;
	    break;

	default:

	    logoutput_debug("%s: error ... type %u not supported", __FUNCTION__, type);

    }

}

unsigned char IO_object_valid(struct io_object_s *object)
{
    unsigned char result=0;

    if (object==NULL) return 0;

    switch (object->type) {

	case IO_OBJECT_TYPE_CONNECTION:

	    result=IO_connection_object_valid(&object->io.connection);
	    break;

	case IO_OBJECT_TYPE_SYSTEM:

	    result=IO_system_object_valid(&object->io.system);
	    break;

	default:

	    logoutput_debug("%s: error ... type %u not supported", __FUNCTION__, object->type);

    }

    return result;
}

void IO_object_clear(struct io_object_s *object)
{
    memset(object, 0 ,sizeof(struct io_object_s));
}

unsigned char IO_object_copy(struct io_object_s *ao, struct io_object_s *bo, unsigned char cb)
{

    unsigned char result=0;

    if ((ao==NULL) || (bo==NULL)) return 0;
    if (ao->type != bo->type) return 0;

    switch (ao->type) {

	case IO_OBJECT_TYPE_CONNECTION:

	    result=IO_connection_object_copy(&ao->io.connection, &bo->io.connection, cb);
	    break;

	case IO_OBJECT_TYPE_SYSTEM:

	    result=IO_system_object_copy(&ao->io.system, &bo->io.system, cb);
	    break;

	default:

	    logoutput_debug("%s: error ... type %u not supported", __FUNCTION__, ao->type);

    }

    return result;

}

unsigned char IO_object_close(struct io_object_s *ao)
{
    return (IO_object_valid(ao) ? IO_object_backend_close(ao->backend) : 0);
}

unsigned char IO_object_is_open(struct io_object_s *ao)
{
    return (IO_object_valid(ao) ? IO_object_backend_is_open(ao->backend) : 0);
}

unsigned char IO_object_get_error(struct io_object_s *ao, struct error_s *error)
{
    return ((IO_object_valid(ao)) ? IO_object_backend_get_error(ao->backend, error) : 0);
}

int IO_object_get_unix_fd(struct io_object_s *ao)
{
    return ((IO_object_valid(ao)) ? IO_object_backend_get_unix_fd(ao->backend) : -1);
}

void IO_object_set_unix_fd(struct io_object_s *ao, int fd)
{
    if (IO_object_valid(ao)) IO_object_backend_set_unix_fd(ao->backend, fd);
}

int IO_object_get_properties(struct io_object_s *ao, struct io_object_properties_s *prop, unsigned char remote)
{
    int result=-1;

    if ((ao==NULL) || (prop==NULL)) return -1;

    switch (ao->type) {

	case IO_OBJECT_TYPE_CONNECTION:

	    result=IO_connection_object_get_properties(&ao->io.connection, prop, remote);
	    break;

	default:

	    logoutput_debug("%s: error ... type %u not supported", __FUNCTION__, ao->type);

    }

    return result;
}
