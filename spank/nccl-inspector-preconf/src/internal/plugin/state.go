package plugin

type State struct {
	InspectorSO string

	LogDir            string
	LogDirSetByPlugin bool
}

func NewState() *State {
	return &State{}
}
