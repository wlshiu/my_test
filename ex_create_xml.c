#include <stdio.h>
#include <stdlib.h>

#include "libxml/parser.h"

void create_doc(char *path)
{
    xmlDocPtr doc;
    xmlNodePtr root;
    xmlNodePtr node;
    xmlNodePtr node2;
    xmlChar *buf;
    int len;

    /* new doc */
    doc = xmlNewDoc(NULL);
    root = xmlNewNode(NULL, BAD_CAST("network"));
    xmlDocSetRootElement(doc, root);

    /* 加入name */
    node = xmlNewNode(NULL, BAD_CAST("name"));
    node2 = xmlNewText(BAD_CAST("s-engine-net"));
    xmlAddChild(node, node2);
    xmlAddChild(root, node);

    /* 加入bridge */
    node = xmlNewNode(NULL, BAD_CAST("bridge"));
    xmlNewProp(node, BAD_CAST("name"), BAD_CAST("s-engine-br0"));
    xmlNewProp(node, BAD_CAST("stp"), BAD_CAST("on"));
    xmlNewProp(node, BAD_CAST("delay"), BAD_CAST("0"));
    xmlAddChild(root, node);

    /* 加入ip */
    node = xmlNewNode(NULL, BAD_CAST("ip"));
    xmlNewProp(node, BAD_CAST("address"), BAD_CAST("172.16.0.1"));
    xmlNewProp(node, BAD_CAST("netmask"), BAD_CAST("255.255.0.0"));
    xmlAddChild(root, node);
    node2 = xmlNewNode(NULL, BAD_CAST("dhcp"));
    xmlAddChild(node, node2);
    node = xmlNewNode(NULL, BAD_CAST("range"));
    xmlNewProp(node, BAD_CAST("start"), BAD_CAST("172.16.100.100"));
    xmlNewProp(node, BAD_CAST("end"), BAD_CAST("172.16.100.254"));
    xmlAddChild(node2, node);


    /* 輸出到記憶體 */
    xmlDocDumpFormatMemory(doc, &buf, &len, 1);
    if (len > 0)
    {
        printf("%s\n", buf);
        free(buf);
    }

    /* 輸出到檔案 */
    if (xmlSaveFormatFile(path, doc, 1) == -1)
    {
        fprintf(stderr, "failed to save file \"%s\"\n.", path);
    }

    /* 釋放記憶體 */
    xmlFreeDoc(doc);

    return;
}

//int main(int argc, char **argv)
//{
//
//    if (argc < 2)
//    {
//        fprintf(stderr, "Usage: %s filename\n", argv[0]);
//        return -1;
//    }
//
//    create_doc(argv[1]);
//    return 0;
//}
