#include "catalog.h"


const Status RelCatalog::createRel(const string & relation, 
		const int attrCnt,
		const attrInfo attrList[])
{
	Status status;
	RelDesc rd;
	AttrDesc ad;

	if (relation.empty() || attrCnt < 1)
		return BADCATPARM;

	if (relation.length() >= sizeof rd.relName)
		return NAMETOOLONG;

	status = getInfo(relation,rd);
	if(status == OK)
	{
		return RELEXISTS;
	}
	else if(status !=RELNOTFOUND)
	{
		return status;
	}
	memcpy(rd.relName,relation.c_str(),sizeof(rd.relName));
	rd.attrCnt = attrCnt;
	status = addInfo(rd);
	if(status!=OK)
	{
		return status ;
	}
	int i ;
	int offset = 0 ;
	int size = 0 ;
	for(i=0 ;i<attrCnt;i++)
	{
		size += attrList[i].attrLen;
	}
	if(size>PAGESIZE)
	{
		return ATTRTOOLONG ;
	}
	memcpy(ad.relName,attrList[i].relName,sizeof(ad.relName));

	for (i = 0 ; i<attrCnt ; i++)
	{
		status = attrCat->getInfo(relation,string(attrList[i].attrName),ad);
		if(status==ATTRNOTFOUND)
		{
			ad.attrLen =attrList[i].attrLen;
			if((strlen(attrList[i].attrName)+1)>sizeof(ad.attrName))
			{
				return NAMETOOLONG ;
			}
			memcpy(ad.attrName, attrList[i].attrName,sizeof(ad.attrName));
			ad.attrOffset = offset ;
			offset+=ad.attrLen ;
			ad.attrType = attrList[i].attrType;
			status = attrCat->addInfo(ad);
			if(status !=OK)
			{
				return status ;
			}
		}
		else if(status !=OK)
		{
			return status ;
		}
		else
		{
			return DUPLATTR;
		}
	}
	status = createHeapFile(relation);
	return status ;

}

