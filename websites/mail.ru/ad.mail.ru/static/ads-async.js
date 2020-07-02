/**
 * Ads.js - Mail.ru external ads
 * Release date: 03.04.2020
 * Version: 1.0.13
 */
(function(c,g,B){var w;w={l:function(c){return g.getElementById(c)},a:function(c,g,p){c.addEventListener?c.addEventListener(g,p,!1):c.attachEvent&&c.attachEvent("on"+g,p)},b:function(c,g,p){c.removeEventListener?c.removeEventListener(g,p,!1):c.detachEvent&&c.detachEvent("on"+g,p)},m:function(c){var g=0,p;for(p in c)c.hasOwnProperty(p)&&g++;return g}};(function(C,l){function p(){this.push=function(){for(var b=0;b<arguments.length;b++){var a=arguments[b];try{a&&(a.iframeMode&&"boolean"===typeof a.iframeMode&&
(X=!0),a.onAdsLoaded&&"function"===typeof a.onAdsLoaded&&(f.onAdsLoaded=a.onAdsLoaded),a.onAdsSuccess&&"function"===typeof a.onAdsSuccess&&(f.onAdsSuccess=a.onAdsSuccess),a.onAdsRefreshed&&"function"===typeof a.onAdsRefreshed&&(f.onAdsRefreshed=a.onAdsRefreshed),a.onAdsClicked&&"function"===typeof a.onAdsClicked&&(f.onAdsClicked=a.onAdsClicked),a.onAdsClosed&&"function"===typeof a.onAdsClosed&&(f.onAdsClosed=a.onAdsClosed),a.onNoAds&&"function"===typeof a.onNoAds&&(f.onNoAds=a.onNoAds),a.onScriptError&&
"function"===typeof a.onScriptError&&(f.onScriptError=a.onScriptError),a.preventDefaultRendering&&"boolean"===typeof a.preventDefaultRendering&&(Y=!0),"boolean"===typeof a.preventAutoLoad&&(U=a.preventAutoLoad),"boolean"===typeof a.disableCollect&&(Z=a.disableCollect)),!a.element&&U||aa(a.element)}catch(d){}}};l.a(c,"load",ba);l.a(c,"orientationchange",r);l.a(c,"resize",r);l.a(g,"mrg-click",ca);l.a(g,"mrg-close",da)}function aa(b){D=b?[b]:g.querySelectorAll("ins.mrg-tag");for(var a,d=0,c=D.length;d<
c;d++)if(a=D[d],!a.getAttribute("data-ad-status")||b)a.id=B+"_"+a.getAttribute("data-ad-slot")+"_"+(new Date).getTime(),a.setAttribute("data-ad-status","pending"),X?x(a):I(a)}function I(b){var a;b?(a=b,J.push(b)):a=J&&J[0];a&&(b=a&&a.getAttribute("data-ad-slot"),a=y(a,"ad-query")||"",E||(clearTimeout(E),E=setTimeout(O,3E3),u([b],a)))}function O(){clearTimeout(E);P.warn("Seems to be connection problems")}function x(b){var a,d,c,e;a=b&&b.getAttribute("data-ad-slot");e=y(b,"ad-query")||"";d=w("?"+e);
e=e.replace("udid=",(/(iPad|iPhone|iPod)/g.test(navigator.userAgent)?"idfa":d&&0<=d.indexOf("-")?"advertising_id":"sha1_android_id")+"=").replace("lng=","lon=");c="https://ad.mail.ru/mobile/"+a+"/?"+e;(function(b,a){la(c,function(d){d?(d={html:d,slot:a},F(d,b),f.onAdsLoaded&&f.onAdsLoaded(d),b.setAttribute("data-ad-status","ready")):(f.onNoAds&&f.onNoAds({slot:a,el:b}),b.setAttribute("data-ad-status","empty"))})})(b,a)}function u(b,a){var d=a;Q.length&&(d="last_shown_ids="+Q.join(",")+(a?"&"+a:""));
if(!Z){var c=ma();c&&(d+=(d?"&":"")+"title="+c)}var e="https://ad.mail.ru/adq/?callback="+ea+("&q="+b.join("&q="))+(d?"&"+d:"")+"&_="+Math.round(1E9*Math.random());(function(b){na(e,function(a){a:{if(f.onAdsLoaded&&"function"===typeof f.onAdsLoaded&&(f.onAdsLoaded(a),Y))break a;if((!a||a&&!a.length)&&f.onNoAds&&"function"===typeof f.onNoAds)f.onNoAds();for(var d=a.length-1;0<=d;d--)if(a[d].html&&/\S/.test(a[d].html)||!f.onNoAds||"function"!==typeof f.onNoAds){var c=F(a[d],b);c&&("string"===typeof c&&
(c=c.replace("b","").split("_")[0]),Q=Q.concat(c));b.setAttribute("data-ad-status","ready");if(f.onAdsSuccess&&"function"===typeof f.onAdsSuccess)f.onAdsSuccess({slot:a[d].slot,el:b,data:a[d]})}else f.onNoAds({slot:a[d].slot,el:b}),b.setAttribute("data-ad-status","empty");J.shift();clearTimeout(E);E=0;I()}})})(J[0])}function F(b,a){var d=new RegExp(fa,"img"),c=g.write,e=b.html,f;a.innerHTML=e;e=K(a,e);if(f=a.querySelector(".trg-b-standard.floating")){if(e=L(a))return a.innerHTML="",!1;z(f);m=a;k(a);
A(a)}else if(f=a.querySelector(".trg-b-standard"))a.innerHTML="",d=document.createElement("iframe"),d.style.width="100%",d.style.height="0",d.style.border="none",d.marginWidth=d.marginHeight="0",a.appendChild(d),d.contentWindow.document.open(),d.contentWindow.document.write(e),d.contentWindow.document.close(),d.style.transition=R.transition,d.style.webkitTransition=R.webkitTransition,d.style.height=d.contentWindow.document.body.scrollHeight+"px";else try{M=e.match(d),g.write=function(){},M&&S(a),
g.write=c}catch(h){P.warn("Can't evaluate script")}e=[];if((d=a.querySelectorAll("[data-trg-id]"))&&d.length)for(c=0;c<d.length;c++){var l=d[c].getAttribute("data-trg-id");l&&e.push(l)}if(e.length)return e;if(f&&f.id)return f.id}function K(b,a){var d;d=b.querySelector(".trg-b-standard");if(!d)return a;var c=[],e=b.querySelectorAll(".mailru-visibility-check-counter");(d=d&&d.getAttribute("data-counter"))&&c.push(d);for(d=e.length-1;0<=d;d--){var f=e[d].getAttribute("data-counter");f&&c.push(f)}a=a.replace("data-counter",
"data-counter-blocker");b.innerHTML=a;if(c.length)try{C.j({h:b,o:c})}catch(g){P.log(g)}return a}function L(b){var a=g.location.href+"_"+b.id.replace(B,"");b="object"===typeof localStorage&&localStorage.setItem?localStorage.getItem(a.toString()):null;b=(new Date).getTime()-(b||0);if(6E4<=b)return!1;setTimeout(function(){V(a)},6E4-b);return!0}function z(b){if(T=b.className.split("timeout_")[1])T=parseInt(T.split(" ")[0],10),ga=setTimeout(function(){var a=m.id.replace(B,"");if(f.onAdsRefreshed&&"function"===
typeof f.onAdsRefreshed)f.onAdsRefreshed(a);u([a])},1E3*T)}function k(b){if(b&&b.parentNode){var a=b.innerHTML;b.innerHTML="";b.parentNode.removeChild(b);g.getElementsByTagName("body")[0].insertBefore(b,g.getElementsByTagName("body")[0].firstChild);var d=document.createElement("iframe");d.style.width=d.style.height="100%";d.marginWidth=d.marginHeight="0";d.scrolling="no";b.appendChild(d);d.contentWindow.document.open();d.contentWindow.document.write(a);d.contentWindow.document.close();b.style.height=
b.clientHeight+"px";b.style.background="#fff";n();b.appendChild(N());b.querySelector(".standard_320x50")&&(N().style.right="initial",N().style.marginLeft="130px");if(ha=0===b.offsetTop)b.style.bottom="auto",b.style.position="absolute";l.a(c,"touchstart",G);l.a(c,"touchmove",G);l.a(c,"scroll",v);l.a(c,"touchend",q)}}function n(){for(var b in R)m.style[b]=R[b]}function r(b,a){for(var d,c=D.length-1;0<=c;c--)if(D[c]!==m&&(d=D[c].getElementsByTagName("iframe")[0])){try{a=frameElement.contentWindow.document?
!0:!1}catch(e){a=!1}a&&(b=d.getClientRects()[0].height,d.style.height="0",function(a){setTimeout(function(){try{a.style.height=a.contentWindow.document.body.scrollHeight+"px"}catch(d){a.style.height=b+"px"}},10)}(d))}q()}function q(){clearTimeout(H);var b=oa(),a=m&&m.firstChild,d=1/b,e=100/d;if(a){if(1<b||ha){m.style.position="absolute";m.style.top="0";m.style.left="0";m.style.left=g.body.scrollLeft*b+"px";m.style.width=c.innerWidth*b+"px";m.style.zoom=1/b;var f=g.documentElement,h=(c.pageYOffset||
f.scrollTop)-(f.clientTop||0);m.style.left=Math.floor(((c.pageXOffset||f.scrollLeft)-(f.clientLeft||0))*b+c.innerWidth*b-m.scrollWidth/b)+"px";m.style.top=Math.floor(h*b+c.innerHeight*b-50)+"px";b=(e-100)/2;a.setAttribute("style",pa({scale:d,translateX:"-"+b+"%",translateY:"-"+b+"%"})+"; width: "+e+"%; height: "+50/d+"px;border:none");H=setTimeout(function(){q()},300)}else a.hasAttribute("style")&&a.removeAttribute("style"),a.style.border="none",a.style.width=a.style.height="100%",n();m?(m.style.height=
"48px",m.style.opacity=1):W()}}function G(b,a){b=b||c.event;(a=b.target||b.srcElement)&&!m.contains(a)&&(clearTimeout(H),e())}function v(){e();clearTimeout(H);H=setTimeout(q,1E3)}function A(b){b.style.borderTop="1px solid #AAA";b.style.boxShadow="0px 0px 5px 0px rgba(0, 0, 0, 0.3)"}function N(){t||(t=new Image,t.src="//r.mradx.net/img/DC/AADF4A.png",t.style.position="absolute",t.style.top="-15px",t.style.right="0",t.style.width="25px",t.style.padding="5px",l.a(t,"click",h));return t}function h(){W();
clearTimeout(ga);clearTimeout(H);e(!0);var b=m.id.replace(B,""),a=g.location.href+"_"+b;V(a);clearInterval(ia);ia=setInterval(function(){V(a)},6E4);if(f.onAdsClosed&&"function"===typeof f.onAdsClosed)f.onAdsClosed({slot:b,el:m})}function e(b){m?(m.style.height="0px",m.style.opacity=0,b&&function(a){setTimeout(function(){try{a.parentNode.removeChild(a),m=null}catch(b){}},1E3)}(m)):W()}function ca(b){if(b.target&&(b=b.target,f.onAdsClicked&&"function"===typeof f.onAdsClicked))f.onAdsClicked({slot:y(b,
"ad-slot"),link:y(b,"ad-link"),w:y(b,"ad-slot")})}function da(b){if(b.target&&(b=b.target,f.onAdsClosed&&"function"===typeof f.onAdsClosed))f.onAdsClosed({slot:y(b,"ad-slot"),el:b})}function ba(){U||aa();q()}function W(){l.b(c,"load",ba);l.b(c,"touchstart",G);l.b(c,"touchmove",G);l.b(c,"scroll",v);l.b(c,"touchend",q);l.b(c,"orientationchange",r);l.b(c,"resize",r);l.b(g,"mrg-click",ca);l.b(g,"mrg-close",da)}function w(b){var a;a="udid".replace(/[\[]/,"\\[").replace(/[\]]/,"\\]");b=(new RegExp("[\\?&]"+
a+"=([^&#]*)")).exec(b);return null===b?"":decodeURIComponent(b[1].replace(/\+/g," "))}function ma(){var b=document.title;return b&&"string"===typeof b?encodeURIComponent(b.substring(0,256)):""}function pa(b){b=b||{};var a="",d;for(d in b)a+=d+"("+b[d]+") ";a+="translateZ(0)";return"-webkit-transform: "+a+"; -moz-transform: "+a+"; transform: "+a}function na(b,a){c[ea]=function(b){a(b)};var d=g.createElement("script");d.src=b;g.getElementsByTagName("head")[0].appendChild(d)}function y(b,a){if(b)return b.getAttribute("data-"+
a);P.warn("No element to get attribute");return null}function S(b){if(M&&M.length){var a=M.shift(),d=a.match(new RegExp(fa,"im"))[1],c=/<script[^>]+src=(\"(.*?)\"|\'(.*?)\'|(.*?))>/gi.exec(a);if(c=c&&(c[4]||c[3]||c[2]||""))-1<a.indexOf("async")?(ja(c),S(b)):ja(c,function(){S(b)});else{try{eval(d)}catch(e){if(f.onScriptError&&"function"===typeof f.onScriptError)f.onScriptError({el:b,slot:y(b,"ad-slot"),error:e})}S(b)}}}function qa(){var b;try{b=new ActiveXObject("Msxml2.XMLHTTP")}catch(a){try{b=new ActiveXObject("Microsoft.XMLHTTP")}catch(d){b=
!1}}b||"undefined"===typeof XMLHttpRequest||(b=new XMLHttpRequest);return b}function la(b,a){var d=qa();d.open("GET",b,!0);d.onreadystatechange=function(){4===d.readyState&&200===d.status&&a(d.responseText)};d.send(null)}function ja(b,a){var d=g.createElement("script"),c;d.setAttribute("src",b);a&&(d.onreadystatechange=d.onload=function(){c||a();c=!0});try{g.getElementsByTagName("head")[0].appendChild(d)}catch(e){}}function oa(){var b=/(iPad|iPhone|iPod)/g.test(navigator.userAgent),a,d=90===Math.abs(c.orientation);
320===c.screen.width?a=d?480:320:a=c.screen[d?"height":"width"];return a/(b?c.innerWidth:c[d?"innerHeight":"innerWidth"])}function V(b){var a=(new Date).getTime();"object"===typeof localStorage&&localStorage.setItem&&localStorage.setItem(b.toString(),a.toString())}var P=c.console||{log:function(){}};if("[object Object]"!==Object.prototype.toString.call(c.MRGtag)){var ka,D=[],f={},J=[],Y=!1,X=!1,U=!1,Z=!1,ea=B+(new Date).getTime(),Q=[],E=0,R={position:"fixed",width:"100%",zoom:1,top:"auto",left:"auto",
bottom:"0",textAlign:"center",zIndex:"2147483647",transition:"opacity .3s,height .3s",webkitTransition:"opacity .3s,height .3s"},m,T,ga,H,ia,ha,t;ka=c.MRGtag;c.MRGtag=new p;c.MRGtag.push.apply(c.MRGtag,ka);var fa="(?:<script.*?>)((\n|\r|.)*?)(?:\x3c/script>)",M}})(function(C){function l(){z=!0;x(!0)}function p(){z=!1}function w(){g.hidden?z=!1:l()}function I(){x()}function O(){x()}function x(h){for(var e in k)k[e]&&50<=K(k[e].g)&&F()&&!k[e].t?function(e){k[e].t=setTimeout(function(){try{var h,n;if(!G(e))u(e);
else if(k[e])if(h=k[e].g,F())if(50>K(h))u(e,!0);else{for(n=k[e].i.length-1;0<=n;n--)(new Image).src=k[e].i[n];u(e);N(k)||(A(c,"resize",O),A(c,"scroll",I),r&&A(g,r,w),A(c,"focus",l),A(c,"blur",p),clearTimeout(q),L=!1)}else u(e,!0)}catch(v){}},1E3)}(e):k[e]&&50>K(k[e].g)?k[e].t&&(clearTimeout(k[e].t),delete k[e].t):k[e]&&!F()?k[e].t&&(clearTimeout(k[e].t),delete k[e].t,clearTimeout(q),h=!1):k[e]||u(e);h&&(clearTimeout(q),q=setTimeout(function(){x(!0)},300))}function u(c,e){if(c&&k[c])clearTimeout(k[c].t),
k[c].t=null,e||(k[c].g=null,delete k[c]);else{clearTimeout(q);q=null;for(var g in k)k[g].t&&u(g,!0)}}function F(){r||"function"!==typeof g.hasFocus||(z=g.hasFocus());return z}function K(h){var e={};e.f=e.c=Infinity;!isNaN(g.body.clientWidth)&&0<g.body.clientWidth&&(e.f=g.body.clientWidth);!isNaN(g.body.clientHeight)&&0<g.body.clientHeight&&(e.c=g.body.clientHeight);n&&n.clientWidth&&!isNaN(n.clientWidth)&&(e.f=n.clientWidth);n&&n.clientHeight&&!isNaN(n.clientHeight)&&(e.c=n.clientHeight);c.innerWidth&&
!isNaN(c.innerWidth)&&(e.f=Math.min(e.f,c.innerWidth));c.innerHeight&&!isNaN(c.innerHeight)&&(e.c=Math.min(e.c,c.innerHeight));if(Infinity===e.c||Infinity===e.c)e={E:":-("};else{h=h.getClientRects()[0];if(!h)return e={p:0};e.t=h.top;e.s=h.bottom;e.v=h.left;e.r=h.right;e=0>h.bottom||0>h.right||h.top>e.clientHeight||h.left>e.clientWidth?0:Math.round((Math.floor(Math.min(e.f,h.right))-Math.ceil(Math.max(0,h.left)))*(Math.floor(Math.min(e.c,h.bottom))-Math.ceil(Math.max(0,h.top)))/((h.right-h.left)*(h.bottom-
h.top))*100)}return e}var L=!1,z=!0,k={},n=g.documentElement,r=g.mozVisibilityState?"mozvisibilitychange":g.webkitVisibilityState?"webkitvisibilitychange":g.visibilityState?"visibilitychange":"",q,G=C.l,v=C.a,A=C.b,N=C.m;return{j:function(h,e){h.h.id||(e=B+(new Date).getTime(),h.h.id=e);e=h.h.id;k[e]||(k[e]={g:h.h,i:h.o});L||(v(c,"resize",O),v(c,"scroll",I),r?v(g,r,w):(v(c,"blur",p),v(c,"focus",l)),L=!0);x(!0)},u:k}}(w),w);"function"===typeof define&&define.amd&&define("mrg/ads",function(){return c.MRGtag})})(window,
document,"mailru_ad");
