#include "catalog.h"


RelCatalog::RelCatalog(Status &status) :
HeapFile(RELCATNAME, status)
{
	// nothing should be needed here
}


const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{
	if (relation.empty())
		return BADCATPARM;

	Status status;
	Record rec;
	RID rid;

	HeapFileScan* tmpPtr =  new HeapFileScan (RELCATNAME,status);
	if(status != OK)
	{
		return status;
	}

	tmpPtr->startScan(0, relation.length()+1,STRING,relation.c_str(),EQ);
	if((status =tmpPtr->scanNext(rid))!=OK)
	{
		delete tmpPtr;
		if(status ==FILEEOF )
		{
			return RELNOTFOUND;
		}
		return status ;
	}
	if((status =tmpPtr->getRecord(rec)) !=OK)
	{
		delete tmpPtr;
		return status ;
	}
	memcpy(&record,rec.data,rec.length);
	status = tmpPtr->endScan();
	if(status!=OK)
	{
		return status;
	}
	delete tmpPtr;
	return OK;
}


const Status RelCatalog::addInfo(RelDesc & record)
{
	RID rid;
	InsertFileScan*  ifs;
	Status status;
	ifs =  new InsertFileScan (RELCATNAME,status);
	if(status!= OK)
	{
		delete ifs;
		return status;
	}
	Record rec;
	memcpy(rec.data,&record,sizeof(RelDesc));
	rec.length = sizeof(RelDesc);

	status = ifs->insertRecord(rec,rid);
	if(status !=OK)
	{

		delete ifs;

		return status;
	}
	delete ifs;
	return OK;
}

const Status RelCatalog::removeInfo(const string & relation)
{
	Status status;
	RID rid;
	HeapFileScan*  hfs;

	if (relation.empty()) return BADCATPARM;
	hfs = new HeapFileScan(RELCATNAME, status);
	if(status!=OK)
	{
		return status;
	}
	if((status=hfs->startScan(0, relation.length()+1,STRING,relation.c_str(),EQ))!=OK)
	{
		return status;

	}
	if((status =hfs->scanNext(rid))!=OK)
	{
		if(status == FILEEOF)
		{
			return RELNOTFOUND;
		}
		return status ;
	}
	if((status =hfs->deleteRecord()) !=OK)
	{
		return status ;
	}
	status = hfs->endScan();
	if(status!=OK)
	{
		return status;
	}
	delete hfs;
	return OK;
}


RelCatalog::~RelCatalog()
{
	// nothing should be needed here
}


AttrCatalog::AttrCatalog(Status &status) :
																			 HeapFile(ATTRCATNAME, status)
{
	// nothing should be needed here
}


const Status AttrCatalog::getInfo(const string & relation, 
		const string & attrName,
		AttrDesc &record)
{

	Status status;
	RID rid;
	Record rec;
	HeapFileScan*  hfs;
	if (relation.empty() || attrName.empty()) return BADCATPARM;

	hfs = new HeapFileScan(ATTRCATNAME,status);
	if(status!=OK)
	{
		return status;
	}

	bool found = false ;

	status = hfs->startScan(0,relation.length()+1,STRING,relation.c_str(),EQ);

	if(status!=OK)
	{
		return status;
	}

	while(!found)
	{
		hfs->scanNext(rid);
		if((status= hfs->getRecord(rec)) != OK)
		{
			return status;
		}

		const char * attr = (char *)((void*)rec.data+MAXNAME);
		const char * attrCompname = attrname;
		if(strcmp(attr, attrName)!=0)
		{

		}
	}





}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
	RID rid;
	InsertFileScan*  ifs;
	Status status;
	Record r;

	ifs = new InsertFileScan(ATTRCATNAME, status);
	if (status != OK)
	{
		delete ifs;
		return status;
	}

	r.data = &record;
	r.length = sizeof(AttrDesc);
	status = ifs->insertRecord(r, rid);
	delete ifs;
	return status;


}


const Status AttrCatalog::removeInfo(const string & relation, 
		const string & attrName)
{
	Status status;
	Record rec;
	RID rid;
	AttrDesc record;
	HeapFileScan*  hfs;
	RelDesc RelDesc;
	if (relation.empty() || attrName.empty()) return BADCATPARM;

	//status = relCat->getInfo(relation,RelDesc);
	//if(status!=OK)
	//{
		//return status;
	//}
	//RelDesc.attrCnt--;

	//	HeapFileScan*  relationScan = new HeapFileScan(RELCATNAME);

	if(status!=OK)
	{
		return status;
	}
	status = hfs->startScan(0,relation.length()+1,STRING,relation.c_str(),EQ);
	while((status = hfs->scanNext(rid))==OK)
	{
		if((status= hfs->getRecord(rec)) != OK)
		{
			return status;
		}
		memcpy(&record,rec.data,sizeof(AttrDesc));
		if(strcmp(record.attrName,attrName.c_str())==0)
		{
			status = hfs->deleteRecord();
			if(status!=OK)
			{
				return status;
			}
			status = hfs->endScan();
			if(status!=OK)
			{
				return status;
			}
			delete hfs;
			return OK;
		}
	}

	if(status == FILEEOF)
	{
		status = hfs->endScan();
		return ATTRNOTFOUND;
	}
	hfs->endScan();

	return status;

}


const Status AttrCatalog::getRelInfo(const string & relation, 
		int &attrCnt,
		AttrDesc *&attrs)
{
	Status status;
	RID rid;
	Record rec;
	HeapFileScan*  hfs;
	int attNum ;
	RelDesc tempDesc;
	status = relCat->getInfo(relation,tempDesc);
	if(status!=OK)
	{
		return status;
	}
	attNum = tempDesc.attrCnt;
	attrs = new AttrDesc[attNum];

	if (relation.empty()) return BADCATPARM;

	hfs = new HeapFileScan(ATTRCATNAME,status);

	if(status!=OK)
	{
		return status;
	}

	status = hfs->startScan(0,relation.length()+1,STRING,relation.c_str(),EQ);
	if(status!=OK)
	{
		return status ;
	}

	while((status=hfs->scanNext(rid))==OK)
	{
		status = hfs->getRecord(rec);
		if(status!=OK)
		{
			return status;
		}
		memcpy(&attrs[attrCnt],rec.data,sizeof(AttrDesc));
		attrCnt++;
	}
	if(status == FILEEOF)
	{
		if(attrCnt == 0)
		{
			return RELNOTFOUND;
		}
	}
	status = hfs->endScan();
	if(status!=OK)
	{
		return status;
	}
	delete hfs;
	return OK;

}


AttrCatalog::~AttrCatalog()
{
	// nothing should be needed here
}

