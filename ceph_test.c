#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rados/librados.h>
#include <inttypes.h>


int main (int argc, const char **argv)
{

        /* Declare the cluster handle and required arguments. */
        rados_t cluster;
        char cluster_name[] = "ceph";
        char user_name[] = "client.admin";
        uint64_t flags = 0;
	int ret = 0;
        /* Initialize the cluster handle with the "ceph" cluster name and the "client.admin" user */
        int err;
        err = rados_create2(&cluster, cluster_name, user_name, flags);

        if (err < 0) {
                fprintf(stderr, "%s: Couldn't create the cluster handle! %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nCreated a cluster handle.\n");
        }


        /* Read a Ceph configuration file to configure the cluster handle. */
        err = rados_conf_read_file(cluster, "/tmp/deployment/ceph.conf");
        if (err < 0) {
                fprintf(stderr, "%s: cannot read config file: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nRead the config file.\n");
        }

        /* Read command line arguments */
        err = rados_conf_parse_argv(cluster, argc, argv);
        if (err < 0) {
                fprintf(stderr, "%s: cannot parse command line arguments: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nRead the command line arguments.\n");
        }

        /* Connect to the cluster */
        err = rados_connect(cluster);
        if (err < 0) {
                fprintf(stderr, "%s: cannot connect to cluster: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nConnected to the cluster.\n");
        }
	
	rados_ioctx_t io;
        char *poolname = "data";

        err = rados_ioctx_create(cluster, poolname, &io);
        if (err < 0) {
                fprintf(stderr, "%s: cannot open rados pool %s: %s\n", argv[0], poolname, strerror(-err));
                rados_shutdown(cluster);
                exit(EXIT_FAILURE);
        } else {
                printf("\nCreated I/O context.\n");
        }
	

        /* Write data to the cluster synchronously. */
        err = rados_write(io, "hw","hello World", 12, 0);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot write object \"hw\" to pool %s: %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nWrote \"Hello World\" to object \"hw\".\n");
        }
	
	//Reading data  From Object
	char buff[100];
	err = rados_read(io, "hw", buff, 12, 0);
	if(err <0){
		 fprintf(stderr, "%s: Cannot read  object \"hw\" to pool %s: %s\n", argv[0], poolname, strerror(-err));
		 rados_ioctx_destroy(io);
                rados_shutdown(cluster);
		exit(1);
        } else {
                printf("\nRead data  from object \"hw\"it is  : %s .\n",buff);
        }

	
	//For storing Key Value Pair
        char xattr[] = "210";
        err = rados_setxattr(io, "pair", "123", xattr,3);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot write xattr to pool %s: %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nWrote Value: \"210\" to xattr key:  \"123\" for object \"pair\".\n");
        }
	char value[100];
	err = rados_getxattr(io, "pair", "123" ,value,100);
	if(err <0){
                 fprintf(stderr, "%s: Cannot read  object \"pair\" to pool %s: %s\n", argv[0], poolname, strerror(-err));
                 rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nRead data value  from object \"pair\" for key 123 it is  : %s .\n",value);
        }

        /*
         * Read data from the cluster asynchronously.
         * First, set up asynchronous I/O completion.
         */
        rados_completion_t comp;
        err = rados_aio_create_completion(NULL, NULL, NULL, &comp);
        if (err < 0) {
                fprintf(stderr, "%s: Could not create aio completion: %s\n", argv[0], strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nCreated AIO completion.\n");
        }
/*	err = rados_aio_write(io, "hw", comp, "Hello World",12,0);
	if (err < 0) {
	        fprintf(stderr, "%s: could not schedule aio write: %s\n", argv[0], strerror(-err));
	        rados_aio_release(comp);
	        rados_ioctx_destroy(io);
	        rados_shutdown(cluster);
	        exit(1);
	}else{
		printf("\nWrote hello world in object hw");
	}


        /* Next, read data using rados_aio_read. */
      /*  char read_res[100];
        err = rados_aio_read(io, "hw", comp, read_res,12,0);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot read object. %s %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nRead object \"hw\". The contents are:\n %s \n", read_res);
        }

        /* Wait for the operation to complete */
       // rados_aio_wait_for_complete(comp);

        /* Release the asynchronous I/O complete handle to avoid memory leaks. */
       // rados_aio_release(comp);


        char xattr_res[100];
        err = rados_getxattr(io, "pair", "123", xattr_res, 3);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot read xattr. %s %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nRead xattr \"key\": 123  for object \"pair\". The contents (values) are:\n %s \n", xattr_res);
        }


//	 uint64_t v = rados_get_last_version(io);
//	printf("%" PRIu64 "\n", v);
	rados_write_op_t write_op = rados_create_write_op();
	    if (!write_op) {
	      printf("failed to allocate write op\n");
	     exit(EXIT_FAILURE);
	     
	    }else{
		printf("Created write_op\n");
		}
		 rados_write_op_assert_exists(write_op);
		
	rados_write_op_create(write_op,LIBRADOS_CREATE_EXCLUSIVE, NULL);

	printf("Created an Object For operation\n");
//	rados_write_op_assert_version(write_op,54);
	const char *keytest = "keytest";
/*	size_t p = 2;
	char *key1 = "key1", *key2 = "key2";
	char *key_arr[2] = {key1, key2};
	int  *vals[2] = {&v1,&v2};
	size_t lens[2] = {4,4};
*/
	size_t p =1;
	char *keys[] = {(char*)"key1"};
	int v1 = 545353;
	char buffer1[100];
	char buffer2[100];
	char buffer3[100];
	int returnValue, buff_size = 50;
	returnValue = snprintf(buffer1, buff_size, " %d  ",v1 );
	printf("%s\n",buffer1);
	char *vals[] = {"value1"};
	 size_t lens[] = {returnValue};
	rados_write_op_omap_set(write_op,keys, vals,lens, p);
	printf("Set the key/value pairs on an object\n");
	err = rados_write_op_operate(write_op, io, keytest, NULL, 0);
	if (err != 0) {
        fprintf(stderr, "rados_write_op_operate failed. oid=%s\n", keytest);
    	}
	rados_release_write_op(write_op);
	
	rados_write_op_omap_rm_keys(write_op,keys, p);
	rados_write_op_operate(write_op, io ,keytest, NULL,0);
	rados_read_op_t read_op = rados_create_read_op();
	
	 rados_omap_iter_t iter_vals;
	int r_vals;
//	rados_read_op_omap_get_vals_by_keys(read_op, key_arr, p,&iter_vals, &r_vals);
	rados_read_op_omap_get_vals2(read_op, NULL, NULL, 100, &iter_vals, NULL, &r_vals);
	rados_read_op_operate(read_op, io,keytest, 0);
	printf("%d\n",r_vals);
	rados_release_read_op(read_op);


	rados_ioctx_destroy(io);
	rados_shutdown(cluster);
}
