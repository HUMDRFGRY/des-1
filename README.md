    	long file_size ,number_of_blocks;
	    srcFile.seekg(0,srcFile.end);
	    size_t size = srcFile.tellg();
	    number_of_blocks=size/8 + ((size%8)?1:0);
	    cout << "File size is : " << size << endl;
	    cout << " Number of blocks is : " << number_of_blocks << endl;
