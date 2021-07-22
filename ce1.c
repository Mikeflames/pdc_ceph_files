#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rados/librados.h>



int set_omap(rados_t cluster, rados_ioctx_t ioctx, const char *oid) {
    int failed = 0;
    int ret,x1,x2,x3,buff_size = 50;
    char buffer1[100],buffer2[100],buffer3[100];
    int v1 = 53423,v2 = 78788 , v3 = 89378;
    // x1 = snprintf(buffer1, buff_size, " %d : Integer value ",v1);
    // x2 = snprintf(buffer2, buff_size, " %d  : Integer value ",v2);
    // x3 = snprintf(buffer3, buff_size, " %d  : Integer value ",v3);
    const char *keys[] = {"key1", "key2", "key3"};
    const char *vals[] = {(char*)&v1, (char*)&v2, (char*)&v3};
    size_t key_lens[] = {5, 5, 5};
    size_t val_lens[] = {4, 4, 4};

    printf("Setting omap on %s ...\n", oid);
    rados_write_op_t wr = rados_create_write_op();
    rados_write_op_create(wr, LIBRADOS_CREATE_IDEMPOTENT, NULL);
    rados_write_op_omap_set2(wr, keys, vals, key_lens, val_lens, 3);
    rados_write_op_write(wr, oid, 4, 0);
    rados_write_op_write_full(wr, "Keys and values in this object", 30);

    ret = rados_write_op_operate(wr, ioctx, oid, NULL, 0);
    if (ret != 0) {
        fprintf(stderr, "rados_write_op_operate failed. oid=%s\n", oid);
        failed = 1;
    }
    rados_release_write_op(wr);
    return failed;
}

int print_one_omap(rados_ioctx_t ioctx, const char *oid) {
    int failed = 0;
    int rval;
    int ret;
    rados_omap_iter_t iter;
    char *k[3];
    char *v[3];
    char b[32];
    size_t bread;
    size_t key_lens[] = {5, 5, 5};
    size_t val_lens[] = {4, 4, 4};

    rados_read_op_t rd = rados_create_read_op();
    rados_read_op_omap_get_vals2(rd, "", "", 1024, &iter, NULL, &rval);
    rados_read_op_read(rd, 0, 32, b, &bread, NULL);

    printf("Printing omap from %s:\n", oid);
    ret = rados_read_op_operate(rd, ioctx, oid, 0);
    if (ret != 0) {
        fprintf(stderr, "rados_read_op_operate failed. oid=%s\n", oid);
        failed = 1;
    } else if (rval != 0) {
        fprintf(stderr, "rval !=0. rval=%d\n", rval);
        failed = 1;
        rados_omap_get_end(iter);
    } else {
        while (1) {
            rados_omap_get_next2(iter, k, v,key_lens, val_lens);
            if (k == NULL || *k == NULL) {
            	 printf("K is NULL");  
		 break;
                       }
        printf("  K:%s  V:%d\n", k[0], *((int*)(v[0])));
        }
    rados_omap_get_end(iter);
    }
    if (bread > 0) {
        printf("  CONTENTS: %s\n", b);
    }

    return failed;
}



/*int print_two_omaps(rados_ioctx_t ioctx, const char *oid1, const char *oid2) {
    int failed = 0;
    int rval;
    int ret;
    rados_omap_iter_t iter;
    char *k;
    char *v;
    char b[32];
    size_t bread;

    rados_read_op_t rd = rados_create_read_op();
    rados_read_op_omap_get_vals2(rd, "", "", 1024, &iter, NULL, &rval);
    rados_read_op_read(rd, 0, 32, b, &bread, NULL);

    bread = 0;
    memset(b, 0, 32);
    printf("Printing omap from %s:\n", oid1);
    ret = rados_read_op_operate(rd, ioctx, oid1, LIBRADOS_OPERATION_IGNORE_CACHE);
    if (ret != 0) {
        fprintf(stderr, "rados_read_op_operate failed. oid=%s\n", oid1);
        failed = 1;
    } else if (rval != 0) {
        fprintf(stderr, "rval !=0. rval=%d\n", rval);
        failed = 1;
    } else {
        while (1) {
            rados_omap_get_next2(iter, &k, &v, NULL, NULL);
            if (k == NULL) {
                break;
            }
            printf("  K:%s  V:%s\n", k, v);
        }
    }
    if (bread > 0) {
        printf("  CONTENTS: %s\n", b);
    }

    bread = 0;
    memset(b, 0, 32);
    printf("Printing omap from %s:\n", oid2);
    ret = rados_read_op_operate(rd, ioctx, oid2, LIBRADOS_OPERATION_IGNORE_CACHE);
    if (ret != 0) {
        fprintf(stderr, "rados_read_op_operate failed. oid=%s\n", oid2);
        failed = 1;
    } else if (rval != 0) {
        fprintf(stderr, "rval !=0. rval=%d\n", rval);
        failed = 1;
    } else {
        while (1) {
            rados_omap_get_next2(iter, &k, &v, NULL, NULL);
            if (k == NULL) {
                break;
            }
            printf("  K:%s  V:%s\n", k, v);
        }
    }
    if (bread > 0) {
        printf("  CONTENTS: %s\n", b);
    }

    rados_omap_get_end(iter);
    return failed;
}
*/
int omap_test(rados_t cluster) {
    char *pool_name = "data";
    int failed = 0;
    int ret;
    rados_ioctx_t ioctx;

    ret = rados_ioctx_create(cluster, pool_name, &ioctx);
    if (ret != 0) {
        fprintf(stderr, "rados_ioctx_create failed\n");
        return 1;
    }

    failed = set_omap(cluster, ioctx, "alpha");
    if (failed) {
        goto done;
    }
    failed = set_omap(cluster, ioctx, "beta");
    if (failed) {
        goto done;
    }

    failed = print_one_omap(ioctx, "alpha");
    if (failed) {printf("Alpha read failed");
        goto done;
    }
    failed = print_one_omap(ioctx, "beta");
    if (failed) {printf("Beta read failed");
        goto done;
    }

  //  failed = print_two_omaps(ioctx, "beta", "alpha");
    //if (failed) {
      //  goto done;
   

	done:
    		rados_ioctx_destroy(ioctx);
    		return failed;
}

int main(void) {
    rados_t cluster;
    int ret;
    int failed = 0;

    ret = rados_create(&cluster, NULL);
    if (ret != 0) {
        failed = 1;
        fprintf(stderr, "rados_create failed\n");
        goto done;
    }
    ret = rados_conf_read_file(cluster, NULL);
    if (ret != 0) {
        failed = 1;
        fprintf(stderr, "rados_conf_read_file failed\n");
        goto done;
    }
    ret = rados_connect(cluster);
    if (ret != 0) {
        failed = 1;
        fprintf(stderr, "rados_connect failed\n");
        goto done;
    }

    failed = omap_test(cluster);
    rados_shutdown(cluster);

done:
    return failed;
}
