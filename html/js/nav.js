// Qiksearch Menu v 1.0
// (c) 2002 Premshree Pillai
// http://www.qiksearch.com
// qiksearch@rediffmail.com

var links = new Array (
  "about",
  "KaliVeda",
  "download",
  "build",
  "kvrootrc",
  "DataAnalysis",
  "Examples",
  "KaliVeda-FAQ",
  "ClassCategories",
  "appli",
  "KaliVedaForum",
  "KaliVedaBugs"
 );

var links_text = new Array (
  "Home",
  "News",
  "Download",
  "Build options",
  "Configuration",
  "Data Analysis",
  "Examples",
  "Tutorials",
  "Class Reference",
  "Commands & GUI",
  "Forum",
  "Report a Bug"
 );

var links_url = new Array (
  "about.html",
  "KaliVeda.html",
  "about.html#download",
  "build.html",
  "kvrootrc.html",
  "DataAnalysis.html",
  "Examples.html",
  "KaliVeda-FAQ.html",
  "ClassCategories.html",
  "appli.html",
  "http://indra.in2p3.fr/KaliVedaForum",
  "https://bugs.launchpad.net/kaliveda/+filebug"
 );

var loc=String(this.location);
loc=loc.split("/");
loc=loc[loc.length-1].split(".");
loc=loc[loc.length-2];

function qiksearch_menu_gen()
{
 for(var i=0; i<links.length; i++)
 {
  if(((this.location=="http://www.qiksearch.com") || (this.location=="http://www.qiksearch.com/") || (this.location=="http://qiksearch.com") || (this.location=="http://qiksearch.com/")) && (i==0))
{
   document.write('<table class="explorer_active" onmouseover="this.className=&#39;explorer_active&#39;;return true" onmouseout="this.className=&#39;explorer_active&#39;;return true" onmousedown="this.className=&#39;explorer_active&#39;;return true" onclick="location.href=&#39;' + links_url[i] + '&#39;"><tr><td><a href="' + links_url[i] + '" class="menu">' + links_text[i] + ' <b>&raquo;</b></a></td></tr></table>');
}
else
{
  if(loc==links[i])
  {
   document.write('<table class="explorer_active" onmouseover="this.className=&#39;explorer_active&#39;;return true" onmouseout="this.className=&#39;explorer_active&#39;;return true" onmousedown="this.className=&#39;explorer_active&#39;;return true" onclick="location.href=&#39;' + links_url[i] + '&#39;"><tr><td><a href="' + links_url[i] + '" class="menu">' + links_text[i] + ' <b>&raquo;</b></a></td></tr></table>');
  }
  else
  {
   document.write('<table class="explorer" onmouseover="this.className=&#39;explorer_over&#39;;return true" onmouseout="this.className=&#39;explorer&#39;;return true" onmousedown="this.className=&#39;explorer_down&#39;;return true" onclick="location.href=&#39;' + links_url[i] + '&#39;"><tr><td><a href="' + links_url[i] + '" class="menu">' + links_text[i] + '</a></td></tr></table>');
  }
 }
  document.write('<table cellspacing="0" cellpadding="0" bgcolor="#FFFFFF"><tr><td></td></tr></table>');
 }
} 

qiksearch_menu_gen();

//function used by 'Search KaliVeda website' form
function send_data(formname)
	{
		formname.q.value+=" site:indra.in2p3.fr inurl:KaliVedaDoc";
		return true;
	}
   