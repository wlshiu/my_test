#include <stdio.h>
#include <stdlib.h>

#include "libxml/parser.h"
#include "libxml/xpath.h"

static void search_doc(xmlXPathContextPtr context, char *search_string)
{
    xmlChar *str = BAD_CAST(search_string);
    xmlXPathObjectPtr result;
    int i;

    printf("search string: \"%s\"\n", search_string);
    result = xmlXPathEval(str, context);
    if (result == NULL || result->nodesetval == NULL)
    {
        printf("  found nothing\n");
        goto err_ret;
    }

    for (i = 0; i < result->nodesetval->nodeNr; i++)
    {
        xmlNodePtr node = result->nodesetval->nodeTab[i];
        if (node->type == XML_ELEMENT_NODE)
        {
            xmlChar *s = xmlNodeListGetString(node->doc, node->children, 1);
            printf("  [%d] element: \"%s\" \"%s\"\n", i, node->name, s);
            free(s);
        }
        else if (node->type == XML_ATTRIBUTE_NODE)
        {
            xmlChar *s = xmlNodeListGetString(node->doc, node->children, 1);
            printf("  [%d] attribute: \"%s\" \"%s\"\n", i, node->name, s);
            free(s);
        }
        else
        {
            printf("  [%d] type %d: \"%s\"\n", i, node->type, node->name);
        }
    }

err_ret:
    if (result != NULL)
        xmlXPathFreeObject(result);
}

int search_xml_node(char *path)
{
    xmlDocPtr doc;
    xmlXPathContextPtr context;

    doc = xmlParseFile(path);
    if (doc == NULL)
    {
        fprintf(stderr, "failed to parse file \"%s\".\n", path);
        return -1;
    }

    context = xmlXPathNewContext(doc);

    /* 測試 */
    search_doc(context, "/network[1]/ip[1]/@address[1] | /network[1]/name[1]");

    /* 這個用於libvirt搜尋domain第一個網路卡MAC地址 */
    /*
       search_doc(context, "/domain[1]/devices[1]/interface[1]/mac[1]/@address[1]");
     */


    /* 釋放資源 */
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return (0);
}
