if [[ ! -e $(which brew) ]]; then
	ruby -e "$(curl -fsSL https://raw.github.com/mxcl/homebrew/go)"
	brew doctor
fi
if [[ ! -e $(which v8) ]]; then
	brew install v8
fi
if [[ ! -e $(which js) ]]; then
	brew install spidermonkey
fi
if [[ ! -e $(which rhino) ]]; then
	brew install rhino
fi
if [[ ! -e /bin/jsc ]]; then
	sudo ln -s /System/Library/Frameworks/JavaScriptCore.framework/Versions/Current/Resources/jsc /bin/jsc
fi
