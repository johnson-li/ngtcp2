(window.webpackJsonpBoomer=window.webpackJsonpBoomer||[]).push([["imdb.IMDbRecentlyViewedReactFeature"],{XAlVyfVDuv:function(e,t,n){"use strict";var r,i=this&&this.__extends||(r=function(e,t){return(r=Object.setPrototypeOf||{__proto__:[]}instanceof Array&&function(e,t){e.__proto__=t}||function(e,t){for(var n in t)t.hasOwnProperty(n)&&(e[n]=t[n])})(e,t)},function(e,t){function n(){this.constructor=e}r(e,t),e.prototype=null===t?Object.create(t):(n.prototype=t.prototype,new n)}),a=this&&this.__assign||function(){return(a=Object.assign||function(e){for(var t,n=1,r=arguments.length;n<r;n++)for(var i in t=arguments[n])Object.prototype.hasOwnProperty.call(t,i)&&(e[i]=t[i]);return e}).apply(this,arguments)};Object.defineProperty(t,"__esModule",{value:!0});var o=n("LDoPTt+kJa"),s=n("qDO/KKoIw4"),c=n("BwmKPM3PmI"),l=function(e){function t(){return null!==e&&e.apply(this,arguments)||this}return i(t,e),t.prototype.render=function(){var e=this.props,t=e.ariaLabel,n=e.id,r=e.href,i=e.name,l=e.posterImage,u=e.type,p=e.watchlistRibbonMetricsContext;return o.createElement(s.PosterCard,{key:n,dynamicWidth:!0},o.createElement(s.PosterCard.Poster,{ariaLabel:t,href:r},"name"!==u&&o.createElement(c.WatchlistConsumer,{id:n,metricsContext:p,render:function(e){return o.createElement(s.Poster.WatchlistRibbon,a({size:"m"},e))}}),o.createElement(s.Poster.Image,a({},l))),o.createElement(s.PosterCard.Title,{href:r,lineClamp:1},i))},t}(o.PureComponent);t.default=l},Y73rO5s0vT:function(e,t,n){"use strict";var r,i=this&&this.__extends||(r=function(e,t){return(r=Object.setPrototypeOf||{__proto__:[]}instanceof Array&&function(e,t){e.__proto__=t}||function(e,t){for(var n in t)t.hasOwnProperty(n)&&(e[n]=t[n])})(e,t)},function(e,t){function n(){this.constructor=e}r(e,t),e.prototype=null===t?Object.create(t):(n.prototype=t.prototype,new n)}),a=this&&this.__assign||function(){return(a=Object.assign||function(e){for(var t,n=1,r=arguments.length;n<r;n++)for(var i in t=arguments[n])Object.prototype.hasOwnProperty.call(t,i)&&(e[i]=t[i]);return e}).apply(this,arguments)},o=this&&this.__awaiter||function(e,t,n,r){return new(n||(n=Promise))(function(i,a){function o(e){try{c(r.next(e))}catch(t){a(t)}}function s(e){try{c(r.throw(e))}catch(t){a(t)}}function c(e){e.done?i(e.value):new n(function(t){t(e.value)}).then(o,s)}c((r=r.apply(e,t||[])).next())})},s=this&&this.__generator||function(e,t){var n,r,i,a,o={label:0,sent:function(){if(1&i[0])throw i[1];return i[1]},trys:[],ops:[]};return a={next:s(0),throw:s(1),return:s(2)},"function"==typeof Symbol&&(a[Symbol.iterator]=function(){return this}),a;function s(a){return function(s){return function(a){if(n)throw new TypeError("Generator is already executing.");for(;o;)try{if(n=1,r&&(i=2&a[0]?r.return:a[0]?r.throw||((i=r.return)&&i.call(r),0):r.next)&&!(i=i.call(r,a[1])).done)return i;switch(r=0,i&&(a=[2&a[0],i.value]),a[0]){case 0:case 1:i=a;break;case 4:return o.label++,{value:a[1],done:!1};case 5:o.label++,r=a[1],a=[0];continue;case 7:a=o.ops.pop(),o.trys.pop();continue;default:if(!(i=(i=o.trys).length>0&&i[i.length-1])&&(6===a[0]||2===a[0])){o=0;continue}if(3===a[0]&&(!i||a[1]>i[0]&&a[1]<i[3])){o.label=a[1];break}if(6===a[0]&&o.label<i[1]){o.label=i[1],i=a;break}if(i&&o.label<i[2]){o.label=i[2],o.ops.push(a);break}i[2]&&o.ops.pop(),o.trys.pop();continue}a=t.call(e,o)}catch(s){a=[6,s],r=0}finally{n=i=0}if(5&a[0])throw a[1];return{value:a[0]?a[1]:void 0,done:!0}}([a,s])}}};Object.defineProperty(t,"__esModule",{value:!0});var c=n("LDoPTt+kJa"),l=n("l42YtCfhhb"),u=n("ekp6onNDDg"),p=n("xGQypn1iJY"),h=n("qDO/KKoIw4"),m=n("iHhLPsdb7V"),d=n("XAlVyfVDuv"),f=n("zFI5+EM4Mo"),y=function(e){function t(t){var n=e.call(this,t)||this;return n.depthQueue=[],n.getPosterImageView=function(e){return{imageModel:e.poster.image,imageType:e.type}},n.state={isContentCleared:!1},n.sendDepthMetrics=l(n.sendDepthMetrics,5e3),n}return i(t,e),t.prototype.render=function(){var e=this.props.RecentlyViewedModel.recentlyViewedItems||[],t=this.state.isContentCleared?[]:e;return c.createElement("div",{className:"recently-viewed"},c.createElement("div",{className:f.subsectionHeadingContainer},c.createElement(h.PageSection.SubSectionTitle,null,"Recently viewed"),t.length>0&&this.renderClearAllLink()),this.renderItems(t))},t.prototype.handleClearAll=function(e){return o(this,void 0,void 0,function(){var t=this;return s(this,function(n){switch(n.label){case 0:return e.preventDefault(),this.emitClearAllMetrics(),[4,fetch("/rvi/_ajax/clear",{method:"POST"}).then(function(e){return e.ok&&t.setState({isContentCleared:!0})})];case 1:return n.sent(),[2]}})})},t.prototype.emitClearAllMetrics=function(){return o(this,void 0,void 0,function(){var e,t;return s(this,function(n){switch(n.label){case 0:return e={ref_:"hm_rvi_clr",pageAction:"clear-all",pt:"home",spt:"main",ht:"actionOnly"},t="/tr/?"+this.encodeParameters(e),[4,fetch(t,{credentials:"same-origin",method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded","Cache-Control":"no-cache"}})];case 1:return n.sent(),[2]}})})},t.prototype.encodeParameters=function(e){return void 0===e&&(e={}),Object.keys(e).map(function(t){return encodeURIComponent(t)+"="+encodeURIComponent(e[t])}).join("&")},t.prototype.renderClearAllLink=function(){var e=this;return c.createElement(h.TextLink,{className:f.clearAllLink,href:"#",text:"Clear all",onClick:function(t){return e.handleClearAll(t)}})},t.prototype.renderItems=function(e){var t=this;return e.length>0?c.createElement(h.Shoveler,{wraps:!1},e.map(function(e,n){return t.renderItem(e,n)})):c.createElement(m.Theme.Consumer,null,function(e){return c.createElement("div",{className:f["emptyStateMessage--"+e.palette.name]},"You have no recently viewed pages")})},t.prototype.renderItem=function(e,t){var n=this;return c.createElement(h.SubGrid.Item,{span:2,key:e.id},c.createElement(p.InView,{triggerOnce:!0,onChange:function(e,r){return n.visibilityChange(t+1)}},c.createElement(d.default,a({},this.getItemView(e)))))},t.prototype.getItemView=function(e){return a({id:e.id,name:e.name,type:e.type},e.poster,{posterImage:this.getPosterImageView(e),watchlistRibbonMetricsContext:e.watchlistRibbonMetricsContext})},t.prototype.visibilityChange=function(e){e>6&&(this.depthQueue.push(e),this.sendDepthMetrics())},t.prototype.sendDepthMetrics=function(){var e=Math.max.apply(Math,this.depthQueue),n={ref_:t.REF_TAG,pageAction:"depth-"+e,pt:t.METRICS_CONTEXT.PAGE_TYPE,spt:t.METRICS_CONTEXT.SUB_PAGE_TYPE,ht:t.METRICS_CONTEXT.HIT_TYPE};u.default({method:"POST",url:t.METRICS_URL,headers:{"Content-type":"application/x-www-form-urlencoded"},data:this.encodeParameters(n)}).catch(function(e){console.error("Failed to submit scroll depth metric.",e)}),this.depthQueue.splice(0,this.depthQueue.length)},t.defaultProps={size:"m"},t.METRICS_URL="/tr/",t.REF_TAG="hm_rvi",t.METRICS_CONTEXT={PAGE_ACTION:"depth-",PAGE_TYPE:"home",SUB_PAGE_TYPE:"main",HIT_TYPE:"actionOnly"},t}(c.Component);t.default=y},jNmNWZemqS:function(e,t,n){"use strict";Object.defineProperty(t,"__esModule",{value:!0});var r=n("Y73rO5s0vT");t.RecentlyViewedFeature=r.default;var i=n("rOeWDqKuSG");t.RecentlyViewedFixture=i.default,t.default=r.default},rOeWDqKuSG:function(e,t,n){"use strict";Object.defineProperty(t,"__esModule",{value:!0});var r=n("Y73rO5s0vT"),i={RecentlyViewedModel:{recentlyViewedItems:[{id:"nm0000553",type:"name",name:"Liam Neeson",poster:{href:"https://www.imdb.com/name/nm0000553/",ariaLabel:"Liam Neeson",image:{caption:"Liam Neeson",maxHeight:783,maxWidth:785,url:"https://m.media-amazon.com/images/M/MV5BMjA1MTQ3NzU1MV5BMl5BanBnXkFtZTgwMDE3Mjg0MzE@._V1_.jpg"}},watchlistRibbonMetricsContext:{pageType:"home",subPageType:"main",refTag:"hm_rvi_wl_rbn"}},{id:"tt0944947",type:"tvSeries",name:"Game of Thrones",poster:{href:"https://www.imdb.com/title/tt0944947/",ariaLabel:"Navigate to Game of Thrones Title Page",image:{url:"https://m.media-amazon.com/images/M/MV5BMjA5NzA5NjMwNl5BMl5BanBnXkFtZTgwNjg2OTk2NzM@._V1_.jpg",maxHeight:1778,maxWidth:1200,caption:"Game of Thrones"}},watchlistRibbonMetricsContext:{pageType:"home",subPageType:"main",refTag:"hm_rvi_wl_rbn"}},{id:"tt1197624",type:"movie",name:"Law Abiding Citizen",poster:{href:"https://www.imdb.com/title/tt1197624/",ariaLabel:"Navigate to Law Abiding Citizen Title Page"},watchlistRibbonMetricsContext:{pageType:"home",subPageType:"main",refTag:"hm_rvi_wl_rbn"}},{id:"tt0386676",type:"tvSeries",name:"The Office",poster:{href:"https://www.imdb.com/title/tt0386676/",ariaLabel:"Navigate to The Office Title Page",image:{url:"https://m.media-amazon.com/images/M/MV5BMTgzNjAzMDE0NF5BMl5BanBnXkFtZTcwNTEyMzM3OA@@._V1_.jpg",maxHeight:2048,maxWidth:1508,caption:"The Office"}},watchlistRibbonMetricsContext:{pageType:"home",subPageType:"main",refTag:"hm_rvi_wl_rbn"}},{id:"tt7366338",type:"tvMiniSeries",name:"Chernobyl",poster:{href:"https://www.imdb.com/title/tt7366338/",ariaLabel:"Navigate to Chernobyl Title Page",image:{url:"https://m.media-amazon.com/images/M/MV5BNTEyYmIzMDUtNWMwNC00Y2Q1LWIyZTgtMGY1YzUxOTAwYTAwXkEyXkFqcGdeQXVyMjIyMTc0ODQ@._V1_.jpg",maxHeight:2048,maxWidth:1382,caption:"Chernobyl"}},watchlistRibbonMetricsContext:{pageType:"home",subPageType:"main",refTag:"hm_rvi_wl_rbn"}},{id:"tt0460649",type:"tvSeries",name:"How I Met Your Mother",poster:{href:"https://www.imdb.com/title/tt0460649/",ariaLabel:"Navigate to How I Met Your Mother Title Page",image:{url:"https://m.media-amazon.com/images/M/MV5BZWJjMDEzZjUtYWE1Yy00M2ZiLThlMmItODljNTAzODFiMzc2XkEyXkFqcGdeQXVyNTA4NzY1MzY@._V1_.jpg",maxHeight:1800,maxWidth:1200,caption:"How I Met Your Mother"}},watchlistRibbonMetricsContext:{pageType:"home",subPageType:"main",refTag:"hm_rvi_wl_rbn"}}]}};t.basicFixture={title:"Feature (defaults)",component:r.default,props:i},t.allFixtures=[t.basicFixture],t.default=t.allFixtures},"zFI5+EM4Mo":function(e,t,n){e.exports={subsectionHeadingContainer:"_22MFdm0dnyBdkT_UczFewS",clearAllLink:"_3QfwxTS4aT8O2Lj9Zd2Cfn","emptyStateMessage--light":"_2zI_z504Xsts0Go5kntfuK","emptyStateMessage--dark":"_3SAg5cHyCOxJ8C1zs2fgyl"}}}]);