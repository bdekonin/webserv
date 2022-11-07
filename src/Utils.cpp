/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkieboom <rkieboom@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/11/05 15:03:05 by rkieboom      #+#    #+#                 */
/*   Updated: 2022/11/07 21:48:32 by bdekonin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Utils.hpp"

ft::RespondCodes::RespondCodes()
{
	// this->response_codes[100] = "Continue";
	// this->response_codes[101] = "Switching protocols";
	// this->response_codes[102] = "Processing";
	// this->response_codes[103] = "Early Hints";

	this->response_codes[200] = "OK";
	// this->response_codes[201] = "Created";
	// this->response_codes[202] = "Accepted";
	this->response_codes[204] = "No Content";
	// this->response_codes[205] = "Reset Content";
	// this->response_codes[206] = "Partial Content";
	// this->response_codes[207] = "Multi-Status";
	// this->response_codes[208] = "Already Reported";
	// this->response_codes[226] = "IM Used";

	this->response_codes[300] = "Multiple Choices";
	this->response_codes[301] = "Moved Permanently";
	this->response_codes[302] = "Found (Previously \"Moved Temporarily\")";
	this->response_codes[303] = "See Other";
	this->response_codes[304] = "Not Modified";
	this->response_codes[305] = "Use Proxy";
	this->response_codes[306] = "Switch Proxy";
	this->response_codes[307] = "Temporary Redirect";
	this->response_codes[308] = "Permanent Redirect";

	this->response_codes[400] = "Bad Request";
	// this->response_codes[401] = "Unauthorized";
	// this->response_codes[402] = "Payment Required";
	this->response_codes[403] = "Forbidden";
	this->response_codes[404] = "Not Found";
	this->response_codes[405] = "Method Not Allowed";
	// this->response_codes[406] = "Not Acceptable";
	// this->response_codes[407] = "Proxy Authentication Required";
	// this->response_codes[408] = "Request Timeout";
	// this->response_codes[409] = "Conflict";
	// this->response_codes[410] = "Gone";
	this->response_codes[411] = "Length Required";
	// this->response_codes[412] = "Precondition Failed";
	this->response_codes[413] = "Request Entity Too Large";
	this->response_codes[414] = "URI Too Long";
	// this->response_codes[415] = "Unsupported Media Type";
	// this->response_codes[416] = "Range Not Satisfiable";
	// this->response_codes[417] = "Expectation Failed";
	// this->response_codes[418] = "I'm a Teapot";
	// this->response_codes[421] = "Misdirected Request";
	// this->response_codes[422] = "Unprocessable Entity";
	// this->response_codes[423] = "Locked";
	// this->response_codes[424] = "Failed Dependency";
	// this->response_codes[425] = "Too Early";
	// this->response_codes[426] = "Upgrade Required";
	// this->response_codes[428] = "Precondition Required";
	// this->response_codes[429] = "Too Many Requests";
	this->response_codes[431] = "Request Header Fields Too Large";
	// this->response_codes[451] = "Unavailable For Legal Reasons";

	this->response_codes[500] = "Internal Server Error";
	// this->response_codes[501] = "Not Implemented";
	// this->response_codes[502] = "Bad Gateway";
	// this->response_codes[503] = "Service Unavailable";
	// this->response_codes[504] = "Gateway Timeout";
	this->response_codes[505] = "HTTP Version Not Supported";
	// this->response_codes[506] = "Variant Also Negotiates";
	// this->response_codes[507] = "Insufficient Storage";
	// this->response_codes[508] = "Loop Detected";
	// this->response_codes[510] = "Not Extended";
	// this->response_codes[511] = "Network Authentication Required";
}

bool ft::RespondCodes::isValidRespondCode(int const &code)
{
	return !this->get_respond_codes()[code].empty();
}

/* Destructor */
ft::RespondCodes::~RespondCodes()
{
}

std::map<int, std::string> ft::RespondCodes::get_respond_codes()
{
	return response_codes;
}

ft::MimeTypes::MimeTypes()
{
	this->mimes["*3gpp"] = "audio/3gpp";
	this->mimes["*jpm"] = "video/jpm";
	this->mimes["*mp3"] = "audio/mp3";
	this->mimes["*rtf"] = "text/rtf";
	this->mimes["*wav"] = "audio/wave";
	this->mimes["*xml"] = "text/xml";
	this->mimes["3g2"] = "video/3gpp2";
	this->mimes["3gp"] = "video/3gpp";
	this->mimes["3gpp"] = "video/3gpp";
	this->mimes["ac"] = "application/pkix-attr-cert";
	this->mimes["adp"] = "audio/adpcm";
	this->mimes["ai"] = "application/postscript";
	this->mimes["apng"] = "image/apng";
	this->mimes["appcache"] = "text/cache-manifest";
	this->mimes["asc"] = "application/pgp-signature";
	this->mimes["atom"] = "application/atom+xml";
	this->mimes["atomcat"] = "application/atomcat+xml";
	this->mimes["atomsvc"] = "application/atomsvc+xml";
	this->mimes["au"] = "audio/basic";
	this->mimes["aw"] = "application/applixware";
	this->mimes["bdoc"] = "application/bdoc";
	this->mimes["bin"] = "application/octet-stream";
	this->mimes["bmp"] = "image/bmp";
	this->mimes["bpk"] = "application/octet-stream";
	this->mimes["buffer"] = "application/octet-stream";
	this->mimes["ccxml"] = "application/ccxml+xml";
	this->mimes["cdmia"] = "application/cdmi-capability";
	this->mimes["cdmic"] = "application/cdmi-container";
	this->mimes["cdmid"] = "application/cdmi-domain";
	this->mimes["cdmio"] = "application/cdmi-object";
	this->mimes["cdmiq"] = "application/cdmi-queue";
	this->mimes["cer"] = "application/pkix-cert";
	this->mimes["cgm"] = "image/cgm";
	this->mimes["class"] = "application/java-vm";
	this->mimes["coffee"] = "text/coffeescript";
	this->mimes["conf"] = "text/plain";
	this->mimes["cpt"] = "application/mac-compactpro";
	this->mimes["crl"] = "application/pkix-crl";
	this->mimes["css"] = "text/css";
	this->mimes["csv"] = "text/csv";
	this->mimes["cu"] = "application/cu-seeme";
	this->mimes["davmount"] = "application/davmount+xml";
	this->mimes["dbk"] = "application/docbook+xml";
	this->mimes["deb"] = "application/octet-stream";
	this->mimes["def"] = "text/plain";
	this->mimes["deploy"] = "application/octet-stream";
	this->mimes["disposition-notification"] = "message/disposition-notification";
	this->mimes["dist"] = "application/octet-stream";
	this->mimes["distz"] = "application/octet-stream";
	this->mimes["dll"] = "application/octet-stream";
	this->mimes["dmg"] = "application/octet-stream";
	this->mimes["dms"] = "application/octet-stream";
	this->mimes["doc"] = "application/msword";
	this->mimes["dot"] = "application/msword";
	this->mimes["drle"] = "image/dicom-rle";
	this->mimes["dssc"] = "application/dssc+der";
	this->mimes["dtd"] = "application/xml-dtd";
	this->mimes["dump"] = "application/octet-stream";
	this->mimes["ear"] = "application/java-archive";
	this->mimes["ecma"] = "application/ecmascript";
	this->mimes["elc"] = "application/octet-stream";
	this->mimes["emf"] = "image/emf";
	this->mimes["eml"] = "message/rfc822";
	this->mimes["emma"] = "application/emma+xml";
	this->mimes["eps"] = "application/postscript";
	this->mimes["epub"] = "application/epub+zip";
	this->mimes["es"] = "application/ecmascript";
	this->mimes["exe"] = "application/octet-stream";
	this->mimes["exi"] = "application/exi";
	this->mimes["exr"] = "image/aces";
	this->mimes["ez"] = "application/andrew-inset";
	this->mimes["fits"] = "image/fits";
	this->mimes["g3"] = "image/g3fax";
	this->mimes["gbr"] = "application/rpki-ghostbusters";
	this->mimes["geojson"] = "application/geo+json";
	this->mimes["gif"] = "image/gif";
	this->mimes["glb"] = "model/gltf-binary";
	this->mimes["gltf"] = "model/gltf+json";
	this->mimes["gml"] = "application/gml+xml";
	this->mimes["gpx"] = "application/gpx+xml";
	this->mimes["gram"] = "application/srgs";
	this->mimes["grxml"] = "application/srgs+xml";
	this->mimes["gxf"] = "application/gxf";
	this->mimes["gz"] = "application/gzip";
	this->mimes["h261"] = "video/h261";
	this->mimes["h263"] = "video/h263";
	this->mimes["h264"] = "video/h264";
	this->mimes["heic"] = "image/heic";
	this->mimes["heics"] = "image/heic-sequence";
	this->mimes["heif"] = "image/heif";
	this->mimes["heifs"] = "image/heif-sequence";
	this->mimes["hjson"] = "application/hjson";
	this->mimes["hlp"] = "application/winhlp";
	this->mimes["hqx"] = "application/mac-binhex40";
	this->mimes["htm"] = "text/html";
	this->mimes["html"] = "text/html";
	this->mimes["ics"] = "text/calendar";
	this->mimes["ief"] = "image/ief";
	this->mimes["ifb"] = "text/calendar";
	this->mimes["iges"] = "model/iges";
	this->mimes["igs"] = "model/iges";
	this->mimes["img"] = "application/octet-stream";
	this->mimes["in"] = "text/plain";
	this->mimes["ini"] = "text/plain";
	this->mimes["ink"] = "application/inkml+xml";
	this->mimes["inkml"] = "application/inkml+xml";
	this->mimes["ipfix"] = "application/ipfix";
	this->mimes["iso"] = "application/octet-stream";
	this->mimes["jade"] = "text/jade";
	this->mimes["jar"] = "application/java-archive";
	this->mimes["jls"] = "image/jls";
	this->mimes["jp2"] = "image/jp2";
	this->mimes["jpe"] = "image/jpeg";
	this->mimes["jpeg"] = "image/jpeg";
	this->mimes["jpf"] = "image/jpx";
	this->mimes["jpg"] = "image/jpeg";
	this->mimes["jpg2"] = "image/jp2";
	this->mimes["jpgm"] = "video/jpm";
	this->mimes["jpgv"] = "video/jpeg";
	this->mimes["jpm"] = "image/jpm";
	this->mimes["jpx"] = "image/jpx";
	this->mimes["js"] = "application/javascript";
	this->mimes["json"] = "application/json";
	this->mimes["json5"] = "application/json5";
	this->mimes["jsonld"] = "application/ld+json";
	this->mimes["jsonml"] = "application/jsonml+json";
	this->mimes["jsx"] = "text/jsx";
	this->mimes["kar"] = "audio/midi";
	this->mimes["ktx"] = "image/ktx";
	this->mimes["less"] = "text/less";
	this->mimes["list"] = "text/plain";
	this->mimes["litcoffee"] = "text/coffeescript";
	this->mimes["log"] = "text/plain";
	this->mimes["lostxml"] = "application/lost+xml";
	this->mimes["lrf"] = "application/octet-stream";
	this->mimes["m1v"] = "video/mpeg";
	this->mimes["m21"] = "application/mp21";
	this->mimes["m2a"] = "audio/mpeg";
	this->mimes["m2v"] = "video/mpeg";
	this->mimes["m3a"] = "audio/mpeg";
	this->mimes["m4a"] = "audio/mp4";
	this->mimes["m4p"] = "application/mp4";
	this->mimes["ma"] = "application/mathematica";
	this->mimes["mads"] = "application/mads+xml";
	this->mimes["man"] = "text/troff";
	this->mimes["manifest"] = "text/cache-manifest";
	this->mimes["map"] = "application/json";
	this->mimes["mar"] = "application/octet-stream";
	this->mimes["markdown"] = "text/markdown";
	this->mimes["mathml"] = "application/mathml+xml";
	this->mimes["mb"] = "application/mathematica";
	this->mimes["mbox"] = "application/mbox";
	this->mimes["md"] = "text/markdown";
	this->mimes["me"] = "text/troff";
	this->mimes["mesh"] = "model/mesh";
	this->mimes["meta4"] = "application/metalink4+xml";
	this->mimes["metalink"] = "application/metalink+xml";
	this->mimes["mets"] = "application/mets+xml";
	this->mimes["mft"] = "application/rpki-manifest";
	this->mimes["mid"] = "audio/midi";
	this->mimes["midi"] = "audio/midi";
	this->mimes["mime"] = "message/rfc822";
	this->mimes["mj2"] = "video/mj2";
	this->mimes["mjp2"] = "video/mj2";
	this->mimes["mjs"] = "application/javascript";
	this->mimes["mml"] = "text/mathml";
	this->mimes["mods"] = "application/mods+xml";
	this->mimes["mov"] = "video/quicktime";
	this->mimes["mp2"] = "audio/mpeg";
	this->mimes["mp21"] = "application/mp21";
	this->mimes["mp2a"] = "audio/mpeg";
	this->mimes["mp3"] = "audio/mpeg";
	this->mimes["mp4"] = "video/mp4";
	this->mimes["mp4a"] = "audio/mp4";
	this->mimes["mp4s"] = "application/mp4";
	this->mimes["mp4v"] = "video/mp4";
	this->mimes["mpd"] = "application/dash+xml";
	this->mimes["mpe"] = "video/mpeg";
	this->mimes["mpeg"] = "video/mpeg";
	this->mimes["mpg"] = "video/mpeg";
	this->mimes["mpg4"] = "video/mp4";
	this->mimes["mpga"] = "audio/mpeg";
	this->mimes["mrc"] = "application/marc";
	this->mimes["mrcx"] = "application/marcxml+xml";
	this->mimes["ms"] = "text/troff";
	this->mimes["mscml"] = "application/mediaservercontrol+xml";
	this->mimes["msh"] = "model/mesh";
	this->mimes["msi"] = "application/octet-stream";
	this->mimes["msm"] = "application/octet-stream";
	this->mimes["msp"] = "application/octet-stream";
	this->mimes["mxf"] = "application/mxf";
	this->mimes["mxml"] = "application/xv+xml";
	this->mimes["n3"] = "text/n3";
	this->mimes["nb"] = "application/mathematica";
	this->mimes["oda"] = "application/oda";
	this->mimes["oga"] = "audio/ogg";
	this->mimes["ogg"] = "audio/ogg";
	this->mimes["ogv"] = "video/ogg";
	this->mimes["ogx"] = "application/ogg";
	this->mimes["omdoc"] = "application/omdoc+xml";
	this->mimes["onepkg"] = "application/onenote";
	this->mimes["onetmp"] = "application/onenote";
	this->mimes["onetoc"] = "application/onenote";
	this->mimes["onetoc2"] = "application/onenote";
	this->mimes["opf"] = "application/oebps-package+xml";
	this->mimes["otf"] = "font/otf";
	this->mimes["owl"] = "application/rdf+xml";
	this->mimes["oxps"] = "application/oxps";
	this->mimes["p10"] = "application/pkcs10";
	this->mimes["p7c"] = "application/pkcs7-mime";
	this->mimes["p7m"] = "application/pkcs7-mime";
	this->mimes["p7s"] = "application/pkcs7-signature";
	this->mimes["p8"] = "application/pkcs8";
	this->mimes["pdf"] = "application/pdf";
	this->mimes["pfr"] = "application/font-tdpfr";
	this->mimes["pgp"] = "application/pgp-encrypted";
	this->mimes["pkg"] = "application/octet-stream";
	this->mimes["pki"] = "application/pkixcmp";
	this->mimes["pkipath"] = "application/pkix-pkipath";
	this->mimes["pls"] = "application/pls+xml";
	this->mimes["png"] = "image/png";
	this->mimes["prf"] = "application/pics-rules";
	this->mimes["ps"] = "application/postscript";
	this->mimes["pskcxml"] = "application/pskc+xml";
	this->mimes["qt"] = "video/quicktime";
	this->mimes["raml"] = "application/raml+yaml";
	this->mimes["rdf"] = "application/rdf+xml";
	this->mimes["rif"] = "application/reginfo+xml";
	this->mimes["rl"] = "application/resource-lists+xml";
	this->mimes["rld"] = "application/resource-lists-diff+xml";
	this->mimes["rmi"] = "audio/midi";
	this->mimes["rnc"] = "application/relax-ng-compact-syntax";
	this->mimes["rng"] = "application/xml";
	this->mimes["roa"] = "application/rpki-roa";
	this->mimes["roff"] = "text/troff";
	this->mimes["rq"] = "application/sparql-query";
	this->mimes["rs"] = "application/rls-services+xml";
	this->mimes["rsd"] = "application/rsd+xml";
	this->mimes["rss"] = "application/rss+xml";
	this->mimes["rtf"] = "application/rtf";
	this->mimes["rtx"] = "text/richtext";
	this->mimes["s3m"] = "audio/s3m";
	this->mimes["sbml"] = "application/sbml+xml";
	this->mimes["scq"] = "application/scvp-cv-request";
	this->mimes["scs"] = "application/scvp-cv-response";
	this->mimes["sdp"] = "application/sdp";
	this->mimes["ser"] = "application/java-serialized-object";
	this->mimes["setpay"] = "application/set-payment-initiation";
	this->mimes["setreg"] = "application/set-registration-initiation";
	this->mimes["sgi"] = "image/sgi";
	this->mimes["sgm"] = "text/sgml";
	this->mimes["sgml"] = "text/sgml";
	this->mimes["shex"] = "text/shex";
	this->mimes["shf"] = "application/shf+xml";
	this->mimes["shtml"] = "text/html";
	this->mimes["sig"] = "application/pgp-signature";
	this->mimes["sil"] = "audio/silk";
	this->mimes["silo"] = "model/mesh";
	this->mimes["slim"] = "text/slim";
	this->mimes["slm"] = "text/slim";
	this->mimes["smi"] = "application/smil+xml";
	this->mimes["smil"] = "application/smil+xml";
	this->mimes["snd"] = "audio/basic";
	this->mimes["so"] = "application/octet-stream";
	this->mimes["spp"] = "application/scvp-vp-response";
	this->mimes["spq"] = "application/scvp-vp-request";
	this->mimes["spx"] = "audio/ogg";
	this->mimes["sru"] = "application/sru+xml";
	this->mimes["srx"] = "application/sparql-results+xml";
	this->mimes["ssdl"] = "application/ssdl+xml";
	this->mimes["ssml"] = "application/ssml+xml";
	this->mimes["stk"] = "application/hyperstudio";
	this->mimes["styl"] = "text/stylus";
	this->mimes["stylus"] = "text/stylus";
	this->mimes["svg"] = "image/svg+xml";
	this->mimes["svgz"] = "image/svg+xml";
	this->mimes["t"] = "text/troff";
	this->mimes["t38"] = "image/t38";
	this->mimes["tei"] = "application/tei+xml";
	this->mimes["teicorpus"] = "application/tei+xml";
	this->mimes["text"] = "text/plain";
	this->mimes["tfi"] = "application/thraud+xml";
	this->mimes["tfx"] = "image/tiff-fx";
	this->mimes["tif"] = "image/tiff";
	this->mimes["tiff"] = "image/tiff";
	this->mimes["tr"] = "text/troff";
	this->mimes["ts"] = "video/mp2t";
	this->mimes["tsd"] = "application/timestamped-data";
	this->mimes["tsv"] = "text/tab-separated-values";
	this->mimes["ttc"] = "font/collection";
	this->mimes["ttf"] = "font/ttf";
	this->mimes["ttl"] = "text/turtle";
	this->mimes["txt"] = "text/plain";
	this->mimes["u8dsn"] = "message/global-delivery-status";
	this->mimes["u8hdr"] = "message/global-headers";
	this->mimes["u8mdn"] = "message/global-disposition-notification";
	this->mimes["u8msg"] = "message/global";
	this->mimes["uri"] = "text/uri-list";
	this->mimes["uris"] = "text/uri-list";
	this->mimes["urls"] = "text/uri-list";
	this->mimes["vcard"] = "text/vcard";
	this->mimes["vrml"] = "model/vrml";
	this->mimes["vtt"] = "text/vtt";
	this->mimes["vxml"] = "application/voicexml+xml";
	this->mimes["war"] = "application/java-archive";
	this->mimes["wasm"] = "application/wasm";
	this->mimes["wav"] = "audio/wav";
	this->mimes["weba"] = "audio/webm";
	this->mimes["webm"] = "video/webm";
	this->mimes["webmanifest"] = "application/manifest+json";
	this->mimes["webp"] = "image/webp";
	this->mimes["wgt"] = "application/widget";
	this->mimes["wmf"] = "image/wmf";
	this->mimes["woff"] = "font/woff";
	this->mimes["woff2"] = "font/woff2";
	this->mimes["wrl"] = "model/vrml";
	this->mimes["wsdl"] = "application/wsdl+xml";
	this->mimes["wspolicy"] = "application/wspolicy+xml";
	this->mimes["x3d"] = "model/x3d+xml";
	this->mimes["x3db"] = "model/x3d+binary";
	this->mimes["x3dbz"] = "model/x3d+binary";
	this->mimes["x3dv"] = "model/x3d+vrml";
	this->mimes["x3dvz"] = "model/x3d+vrml";
	this->mimes["x3dz"] = "model/x3d+xml";
	this->mimes["xaml"] = "application/xaml+xml";
	this->mimes["xdf"] = "application/xcap-diff+xml";
	this->mimes["xdssc"] = "application/dssc+xml";
	this->mimes["xenc"] = "application/xenc+xml";
	this->mimes["xer"] = "application/patch-ops-error+xml";
	this->mimes["xht"] = "application/xhtml+xml";
	this->mimes["xhtml"] = "application/xhtml+xml";
	this->mimes["xhvml"] = "application/xv+xml";
	this->mimes["xm"] = "audio/xm";
	this->mimes["xml"] = "application/xml";
	this->mimes["xop"] = "application/xop+xml";
	this->mimes["xpl"] = "application/xproc+xml";
	this->mimes["xsd"] = "application/xml";
	this->mimes["xsl"] = "application/xml";
	this->mimes["xslt"] = "application/xslt+xml";
	this->mimes["xspf"] = "application/xspf+xml";
	this->mimes["xvm"] = "application/xv+xml";
	this->mimes["xvml"] = "application/xv+xml";
	this->mimes["yaml"] = "text/yaml";
	this->mimes["yang"] = "application/yang";
	this->mimes["yin"] = "application/yin+xml";
	this->mimes["yml"] = "text/yaml";
	this->mimes["zip"] = "application/zip";
}

/* Destructor */
ft::MimeTypes::~MimeTypes()
{
}

/* Get the mime type for a given extension */
std::string ft::MimeTypes::getMimeType(std::string extension)
{
	std::string mime = this->mimes[extension];
	if (mime.empty())
	{
		// mime = "application/octet-stream";
		mime = "text/plain";
	}
	return mime;
}
