================================
Ceph Rados PDC Integration
================================

---------------------------
Connection to PDC_Codebase:
---------------------------
We have Used the Librados Library of Ceph to connect with the PDC server in C language.

* To use Librados, you instantiate a rados_t variable (a cluster handle) and call rados_create() with a pointer to it:

.. code-block:: Bash	

 int retu;
 rados_t cluster;
 retu = rados_create(&cluster, NULL);
 if (retu < 0) {
 fprintf(stderr, "%s: cannot create a cluster handle: %s\n", argv[0], strerror(-err));
 exit(1);
 }

* Then you configure your rados_t to connect to your cluster, either by setting individual values (rados_conf_set()), using a configuration file (rados_conf_read_file()), using command line options (rados_conf_parse_argv()), or an environment variable (rados_conf_parse_env()):

.. code-block:: Bash

 retu = rados_conf_read_file(cluster, NULL);
 if (retu != 0) {
 failed = 1;
 fprintf(stderr, "rados_conf_read_file failed\n");
 goto done;
 }


* Once the cluster handle is configured, you can connect to the cluster with rados_connect():

.. code-block:: Bash

 retu = rados_connect(cluster);
 if (retu < 0) {
 fprintf(stderr, "%s: cannot connect to cluster: %s\n", argv[0], strerror(-err));
 exit(1);
 }

* Then you open an “IO context”, a rados_ioctx_t, with rados_ioctx_create():

.. code-block:: Bash

 retu = rados_ioctx_create(cluster, poolname, &io);
 if (retu != 0) {
 fprintf(stderr, "rados_ioctx_create failed\n");
 return 1;
 }

* Declare Global variables in pdc_server.c file : 

.. code-block:: Bash

 rados_t       cluster;
 rados_ioctx_t io;
 const char *  poolname = "data";

* Finally, you have a connection to the cluster through PDC.
------------------------------------------------
Functions Used for Storing the PDC Server data : 
------------------------------------------------
PDC_Server_rados_write() :
------------------

This function writes the data in 'buf' to rados objects whose names are created as "obj_id_reg_id_batch."Depending on the offset and size values of the global region of PDC, it stores the data in Rados objects inside the pool.
* PDC_Server_rados_write(uint64_t obj_id, void *buf, uint64_t write_size, int ndim, uint64_t *offset,uint64_t *size, int o) :

* PDC_Server_rados_write(uint64_t obj_id, void *buf, uint64_t write_size, int ndim, uint64_t *offset,uint64_t *size, int o) :
	* Input:
		* obj_id is the id coming from PDC objects.
                * buf is the array buffer data that needs to be written on rados objects.
                * write_size is the total size of buf in bytes.
                * ndim is the dimension of the PDC object, which can be 1 or 2 or 3.
                * offset is the value in bytes from where it should start in buf.
                * size is in bytes.
                * o is '1' only for overlap cases, else it is '0'.
               

	* Output:
		* '0' on Success else negative value on failure.
PDC_Server_rados_read():
------------------
This Function reads back the data from the rados objects for the desired call depending on the offset, size, and ndim Value and finally writes the data in buf, which is passed to the PDC Server.

* PDC_Server_rados_read(uint64_t obj_id, void *buf, uint64_t *offset, uint64_t *size)
       * Input:
		* obj_id is the id coming from PDC objects.
                * buf is the array buffer where data read from rados object to be written.
                * offset is the value in bytes from where it should start reading in buf.
                * size is in bytes.
       * Output:
		* '0' on Success else negative value on failure.
--------------------------------
Sub Functions of Librados Used :
--------------------------------
* int rados_write_full(rados_ioctx_tio, constchar*oid, constchar*buf, size_tlen)
Write len bytes from buf into the oid object. The value of len must be <= UINT_MAX/2.
The object is filled with the provided data. If the object exists, it is atomically truncated and then written.

         * Parameters
               * io – the io context in which the write will occur 
	       * oid – the name of the object
               * buf – data to write
               * len – length of the data, in bytes
              
         * Returns : 
               0 on success, negative error code on failure

* int rados_read(rados_ioctx_tio, constchar*oid, char*buf, size_tlen, uint64_toff)
Read data from an object.
The io context determines the snapshot to read from if any was set by rados_ioctx_snap_set_read().

       * Parameters
              * io – the context in which to perform the read
              * oid – the name of the object to read from
              * buf – where to store the results
              * len – the number of bytes to read
              * off – the offset to start reading from in the object

       * Returns :
            the number of bytes read on success, negative error code on failure.
* int rados_setxattr(rados_ioctx_tio, constchar*o, constchar*name, constchar*buf, size_tlen)
Set an extended attribute on an object.
         * Parameters 
                 * io – the context in which xattr is set
                 * o – name of the object
                 * name – which extended attribute to set
                 * buf – what to store in the xattr
                 * len – the number of bytes in buf
        * Returns
                    0 on success, negative error code on failure
* intrados_getxattr(rados_ioctx_tio, constchar*o, constchar*name, char*buf, size_tlen)
Get the value of an extended attribute on an object.
              * Parameters
                       * io – the context in which the attribute is read
                       * o – name of the object
                       * name – which extended attribute to read
                       * buf – where to store the result
                       * len – the size of buf in bytes
              * Returns
                       length of xattr value on success, negative error code on failure.
