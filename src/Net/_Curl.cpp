/*
 * _Curl.cpp
 *
 *  Created on: May 30, 2022
 *      Author: yankai
 */

#include "_Curl.h"

namespace kai
{
	_Curl::_Curl()
	{
		m_cmd = "";
		m_dir = "";
	}

	_Curl::~_Curl()
	{
	}

	bool _Curl::init(void *pKiss)
	{
		IF_F(!this->_FileBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("cmd", &m_cmd);
		pK->v("dir", &m_dir);
		m_dir = checkDirName(m_dir);

		return true;
	}

	bool _Curl::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _Curl::check(void)
	{
		return 0;
	}

	void _Curl::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateFileList();
			updateCurl();

			m_pT->autoFPSto();
		}
	}

	void _Curl::updateFileList(void)
	{
		vector<string> vFile;
		IF_(!getDirFileList(m_dir, &vFile, &m_vExt));

		for (string f : vFile)
		{
			addFile(f);
		}
	}

	void _Curl::updateCurl(void)
	{
		while (!m_vFiles.empty())
		{
			m_fName = m_vFiles.back();
			string cmd = replace(m_cmd, "[fName]", m_fName);

			FILE *fp;
			fp = popen(cmd.c_str(), "r");
			if (!fp)
			{
				LOG_E("Failed to run command: " + cmd);
				continue;
			}

			string strR = "";
			char pResult[1035];
			while (fgets(pResult, sizeof(pResult), fp))
			{
				strR += string(pResult);
			}
			pclose(fp);

			LOG_I(strR);
			if (strR.empty())
			{
				//upload success
				LOG_I("Uploaded: " + m_fName);
				cmd = "rm " + m_fName;
				system(cmd.c_str());
				m_vFiles.pop_back();
			}

			m_fName = "";
		}
	}

	bool _Curl::addFile(const string &fName)
	{
		IF_F(fName.empty());
		IF_T(bFileInList(fName));

		m_vFiles.push_back(fName);
		return true;
	}

	bool _Curl::bFileInList(const string &fName)
	{
		for (string f : m_vFiles)
		{
			IF_T(f == fName);
		}

		return false;
	}

	void _Curl::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_FileBase::console(pConsole);
		IF_(check() < 0);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("Uploading: " + m_fName);
	}

}
