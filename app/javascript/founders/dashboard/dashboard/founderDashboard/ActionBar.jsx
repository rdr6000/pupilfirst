import React from "react";
import PropTypes from "prop-types";
import TargetsFilter from "./TargetsFilter";

export default class ActionBar extends React.Component {
  constructor(props) {
    super(props);

    this.openTimelineBuilder = this.openTimelineBuilder.bind(this);
  }

  openTimelineBuilder() {
    if (this.props.currentLevel == 0) {
      $(".js-founder-dashboard__action-bar-add-event-button").popover("show");

      setTimeout(function () {
        $(".js-founder-dashboard__action-bar-add-event-button").popover("hide");
      }, 3000);
    } else {
      // Open up the timeline builder without any target or timeline event type preselected.
      this.props.setRootState({
        timelineBuilderVisible: true,
        timelineBuilderParams: {
          targetId: null,
          selectedTimelineEventTypeId: null
        }
      });
    }
  }

  componentDidMount() {
    if (this.props.currentLevel == 0) {
      $(".js-founder-dashboard__action-bar-add-event-button").popover({
        title: "Feature Locked!",
        content: "This feature is not available during admission.",
        html: true,
        placement: "bottom",
        trigger: "manual"
      });
    }
  }

  componentWillUnmount() {
    $(".js-founder-dashboard__action-bar-add-event-button").popover("dispose");
  }

  render() {
    return (
      <div className="founder-dashboard-actionbar__container px-2 mx-auto">
        <div className="founder-dashboard-actionbar__box d-flex justify-content-between">
          <TargetsFilter
            getAvailableTrackIds={this.props.getAvailableTrackIds}
            rootProps={this.props.rootProps}
            rootState={this.props.rootState}
            setRootState={this.props.setRootState}
          />

          <div className="d-flex">
            <button
              onClick={this.openTimelineBuilder}
              className="btn btn-with-icon btn-md btn-secondary text-uppercase btn-timeline-builder js-founder-dashboard__trigger-builder d-none d-md-block mr-2 js-founder-dashboard__action-bar-add-event-button"
            >
              <i className="fa fa-plus-circle" aria-hidden="true"/>
              <span>Add Event</span>
            </button>

            <div className="btn-group">
              <button
                className="btn btn-link founder-dashboard-actionbar__show-more-menu dropdown-toggle"
                data-toggle="dropdown"
                type="button"
              >
                <span className="founder-dashboard-actionbar__show-more-menu-dots"/>
              </button>

              <div className="dropdown-menu filter-targets-dropdown__menu dropdown-menu-right">
                <a
                  id="filter-targets-dropdown__tour-button"
                  className="dropdown-item filter-targets-dropdown__menu-item"
                  role="button"
                >
                  Take a Tour
                </a>
              </div>
            </div>
          </div>
        </div>
      </div>
    );
  }
}

ActionBar.propTypes = {
  getAvailableTrackIds: PropTypes.func.isRequired,
  rootProps: PropTypes.object.isRequired,
  rootState: PropTypes.object.isRequired,
  setRootState: PropTypes.func.isRequired
};
