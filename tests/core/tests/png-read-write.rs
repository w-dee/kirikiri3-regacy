// /boot がマウントされているディレクトリを得る

var boot_script_source = File::getFileSystemAt('/boot/').source;

// boot_script_source/../ を /root にマウントする

File::mount('/root', new FileSystem::OSFS("\{boot_script_source}/..", true));

function compareFile(a, b)
{
	return File::open(a) { |st| break st.read() } == File::open(b) { |st| break st.read() };
}

// PNGファイルのファイル名
var filenames = [
	"png256.png",
	"png256a.png",
	"pngA8R8G8B8.png",
	"pngR8G8B8.png",
	"png2.png",
	"png16.png",
	"png16a.png",
	];

// それぞれのPNGファイルに対して
for(var i = 0; i < filenames.length; i++)
{
	var filename = filenames[i];
	System::stderr.print("file \{filename}\n");

	var image = new Image();
	image.load("/root/media/\{filename}");

	var output_filename = File::chopExtension(filename) + ".bmp";
	var dic = new Dictionary();
	dic['_type'] = 'A8R8G8B8'; // ビットマップサブタイプ
	image.save("/root/tmp/\{output_filename}", dic);

	assert(compareFile("/root/media/expected/A8R8G8B8_\{output_filename}", "/root/tmp/\{output_filename}"));
}

// pngR8G8B8 に対して
{
	var filename = "pngR8G8B8.png";
	System::stderr.print("file \{filename}\n");

	var image = new Image();
	image.load("/root/media/\{filename}");

	var dic = new Dictionary();
	dic['_type'] = 'R8G8B8'; // PNG サブタイプ
	image.save("/root/tmp/\{filename}", dic);

	var image = new Image();
	image.load("/root/tmp/\{filename}");

	var output_filename = File::chopExtension(filename) + ".bmp";
	var dic = new Dictionary();
	dic['_type'] = 'R8G8B8'; // ビットマップサブタイプ
	image.save("/root/tmp/\{output_filename}", dic);

	assert(compareFile("/root/media/expected/R8G8B8_\{output_filename}", "/root/tmp/\{output_filename}"));
}

// pngR8G8B8 に対して
{
	var filename = "pngR8G8B8.png";
	System::stderr.print("file \{filename}\n");

	var image = new Image();
	image.load("/root/media/\{filename}");

	var dic = new Dictionary();
	dic['_type'] = 'GRAY8'; // PNG サブタイプ
	image.save("/root/tmp/\{filename}", dic);

	var image = new Image();
	image.load("/root/tmp/\{filename}");

	var output_filename = File::chopExtension(filename) + ".bmp";
	var dic = new Dictionary();
	dic['_type'] = 'R8G8B8'; // ビットマップサブタイプ
	image.save("/root/tmp/\{output_filename}", dic);

	assert(compareFile("/root/media/expected/GRAY8_\{output_filename}", "/root/tmp/\{output_filename}"));
}

// vpAg
{
	var filename = "pngvpAg.png";
	System::stderr.print("file \{filename}\n");

	var image = new Image();
	var dic = new Dictionary();
	image.load("/root/media/\{filename}", dic);

	assert(dic['vpag_w']    == 640);
	assert(dic['vpag_h']    == 480);
	assert(dic['vpag_unit'] == "pixel");
	assert(dic['offs_x']    == 123);
	assert(dic['offs_y']    == 94);
	assert(dic['offs_unit'] == "pixel");

	image.save("/root/tmp/\{filename}", dic);

	var image = new Image();
	image.load("/root/tmp/\{filename}", dic);

	assert(dic['vpag_w']    == 640);
	assert(dic['vpag_h']    == 480);
	assert(dic['vpag_unit'] == "pixel");
	assert(dic['offs_x']    == 123);
	assert(dic['offs_y']    == 94);
	assert(dic['offs_unit'] == "pixel");

}


System::stdout.print("ok"); //=> ok

